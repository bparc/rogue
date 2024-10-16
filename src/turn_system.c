fn void ControlPanel(turn_queue_t *queue, const virtual_controls_t *cons)
{
	if (WentDown(cons->debug01)) // Toggle
		queue->break_mode_enabled = !queue->break_mode_enabled;

	DebugPrint(
		"ACT %i | BRK (F1): %s %s %s",
		(queue->action_points),
		(queue->break_mode_enabled ? "ON" : "OFF"),
		(queue->interp_state == interp_wait_for_input) ? " | STEP (F2) ->" : "",
		(queue->interp_state == interp_wait_for_input) ? interpolator_state_t_names[queue->requested_state] : "");

	if ((queue->interp_state == interp_wait_for_input) && WentDown(cons->debug02))
		queue->request_step = true;
}

fn inline s32 ChangeQueueState(turn_queue_t *queue, interpolator_state_t state)
{
	s32 result = false;
	queue->time = 0.0f;

	#if _DEBUG
	result = (queue->break_mode_enabled == false);
	if (result)
	{
		queue->interp_state = state;
	}
	else
	{
		queue->interp_state = interp_wait_for_input;
		queue->requested_state = state;
		queue->request_step = false;
	}
	#else
	queue->interp_state = state;
	result = true;
	#endif
	return result;
}

fn void TurnKernel(game_world_t *state, entity_storage_t *storage, map_t *map, turn_queue_t *queue, f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out, assets_t *assets)
{
	SetGlobalOffset(out, state->camera_position); // NOTE(): Let's pass the camera position via the PushRenderOutput call instead of this SetGlobalOffset stuff.
	// NOTE(): Process the current turn

	// TODO(Arc): IMPORTANT! Invalidated IDs aren't handled properly by
	// the queue! This has to be fixed when we add the ability to
	// remove an entity from the storage.

	entity_t *entity = NextInOrder(queue, storage);
	if (entity == 0)
	{
		// NOTE(): We run out of the queue, time to schedule new ones.
		EstablishTurnOrder(state, queue, storage);
	}
	if (entity)
	{
		if ((queue->turn_inited == false))
		{
			queue->action_points = BeginTurn(state, entity);
			queue->turn_inited = true;

			queue->interp_state = interp_request;
			queue->time = 0.0f;
			queue->time_elapsed = 0.0f;

			#if ENABLE_TURN_SYSTEM_DEBUG_LOGS
			DebugLog("initiating turn for entity#%i", entity->id);
			#endif
			// NOTE(): Begin the turn.
		}

		Assert(queue->turn_inited);
		// NOTE(): The turn will "stall" until AcceptTurn() is called.

		// NOTE(): DEBUG draw the "discrete" p.
		#if _DEBUG
		RenderIsoTile(out, map, entity->p, Red(), (queue->interp_state == interp_wait_for_input), 0);
		#endif

		// NOTE(): We're focusing the camera either on a cursor or on a player position,
		// depending on the current mode.
		v2 focus_p = state->cursor->active ? GetTileCenter(map, state->cursor->p) : entity->deferred_p;
		state->camera_position = CameraTracking(state->camera_position, focus_p, GetViewport(input), dt);
		
		if (entity->flags & entity_flags_controllable) // NOTE(): The "player" code.
		{
			// NOTE(): Listen for the player input.
			const v2s *considered_dirs = cardinal_directions;
			#if ENABLE_DIAGONAL_MOVEMENT
			if (IsKeyPressed(input, key_code_shift))
				considered_dirs = diagonal_directions;
			#endif
			
			s32 direction = GetDirectionalInput(input);
			b32 input_valid = (direction >= 0) && (direction < 4);
			b32 cursor_mode_active = state->cursor->active; // NOTE(): The cursor_active flag needs to be stored *before* calling DoCursor. This is actually the correct order. For reasons.

			DoCursor(out, entity, cons, input_valid,
				direction, considered_dirs, queue, map, storage, log, state->cursor);
			
			#if _DEBUG // NOTE(): Render the considered directions on the map.
			v2s base_p = cursor_mode_active ? state->cursor->p : entity->p;
			for (s32 index = 0; index < 4; index++)
				RenderIsoTile(out, map, AddS(base_p, considered_dirs[index]), SetAlpha(Orange(), 0.5f), true, 0);
			#endif
			
			//Valid input
			if (input_valid && (cursor_mode_active == false))
			{
				//future position
				v2s peekPos = AddS(entity -> p, considered_dirs[direction]);

				//valid move pos
				if (IsWorldPointEmpty(state, peekPos) && (queue->action_points > 0))
				{
			        MoveEntity(map, entity, considered_dirs[direction]);
			        ApplyTileEffects(peekPos, state, entity);
					// NOTE(): Consume moves
					queue->action_points--;
				}
			}

			// NOTE(): End the turn if we run out of all action points.
			if (queue->action_points <= 0)
						AcceptTurn(queue);
		}
		else
		{
			// NOTE(): Animator
			f32 speed_mul = TURN_SPEED_NORMAL;
			queue->time += dt * speed_mul;
			queue->time_elapsed += dt;

			switch(queue->interp_state)
			{
			case interp_wait_for_input:
				{
					DebugWait(state, entity, queue->requested_state, out);

					if (queue->request_step)
					{
						queue->interp_state = queue->requested_state;
						queue->time = 0.0f;
					}
				} break;
			Request:
			case interp_request:
				{
					// TODO(): Move the player code somewhere around here maybe?
					// It could potentially be a better way to structure this.
					queue->starting_p = entity->p;

					s32 cost = Decide(state, entity);
						queue->action_points -= cost;
					
					if (!ChangeQueueState(queue, interp_transit))
						break;
					queue->time = dt;
				}; // NOTE(): Intentional fall-through. We want to start handling transit immediatly, instead of waiting an addidional frame.
			case interp_transit:
				{
					v2 a = GetTileCenter(map, queue->starting_p);
					v2 b = GetTileCenter(map, entity->p);
					entity->deferred_p = Lerp2(a, b, queue->time);

					if ((queue->time >= 1.0f))
					{
						queue->time = 0.0f;

					 	if (queue->action_points > 0)
					 	{
					 		if (ChangeQueueState(queue, interp_request))
					 			goto Request;
					 	}
					 	else
					 	{
					 		s32 attempt = AttemptAttack(state, entity);
							ChangeQueueState(queue, attempt ? interp_attack : interp_accept);
					 	}
					}
				} break;
			case interp_attack:
				{
					entity_t *target = GetEntity(storage, queue->attack_target);
					b32 finish = true;

					if (target)
					{
						f32 distance = Distance(entity->deferred_p, target->deferred_p);
						f32 t = queue->time / (distance * 0.005f);
						finish = t >= 1.0f;
						AnimateAttack(state, entity, target, t, dt, assets, out, finish);
					}

					if (finish)
						ChangeQueueState(queue, interp_accept);
				} break;
			case interp_accept:
				{
					if (queue->time > 0.1f)
					{
						AcceptTurn(queue);
						#ifdef ENABLE_TURN_SYSTEM_DEBUG_LOGS
						DebugLog("turn finished in %.2f seconds", queue->time_elapsed);
						#endif
					}
				} break;
			}
		}
	}
}