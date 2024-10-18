fn void ControlPanel(turn_queue_t *queue, const virtual_controls_t *cons, entity_storage_t *storage)
{
	if (WentDown(cons->debug01)) // Toggle
		queue->break_mode_enabled = !queue->break_mode_enabled;

	DebugPrint(
		"NUM %i | ACT %i | BRK (F1): %s %s %s",
		(storage->num),
		(queue->action_points),
		(queue->break_mode_enabled ? "ON" : "OFF"),
		(queue->interp_state == interp_wait_for_input) ? " | STEP (F2) ->" : "",
		(queue->interp_state == interp_wait_for_input) ? interpolator_state_t_names[queue->requested_state] : "");

	if ((queue->interp_state == interp_wait_for_input) && WentDown(cons->debug02))
		queue->request_step = true;
}

fn void QueryAsynchronousAction(turn_queue_t *queue, action_type_t type, entity_t *target, v2s target_p)
{
	async_action_t *result = 0;
	if ((queue->action_count < ArraySize(queue->actions)))
	{
		result = &queue->actions[queue->action_count++];
	}
	if (result)
	{
		ZeroStruct(result);
		if (target)
		{
			result->target_id = target->id;
			result->target_p = target->p;
		}
		else
		{
			result->target_p = target_p;
		}
		result->type = type;
	}
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

fn void GarbageCollect(turn_queue_t *queue)
{
	// TODO(): This will mess up the turn oder...
	entity_storage_t *storage = queue->storage;
	#if 1
	for (s32 index = 0; index < storage->num; index++)
	{
		entity_t *entity = &storage->entities[index];
		if (entity->flags & entity_flags_deleted)
			storage->entities[index--] = storage->entities[--storage->num];
	}
	#endif
}

fn void inline ListenForUserInput(entity_t *entity, game_world_t *state, entity_storage_t *storage, map_t *map, turn_queue_t *queue,
	f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out, assets_t *assets)
{
		// NOTE(): Listen for the player input.
		const v2s *directions = cardinal_directions;
		#if ENABLE_DIAGONAL_MOVEMENT
		if (IsKeyPressed(input, key_code_shift))
			directions = diagonal_directions;
		#endif
		
		s32 direction = GetDirectionalInput(input);
		b32 input_valid = (direction >= 0) && (direction < 4);
		b32 cursor_mode_active = state->cursor->active; // NOTE(): The cursor_active flag needs to be stored *before* calling DoCursor. This is actually the correct order. For reasons.
		DoCursor(out, entity, cons, input_valid,
			direction, directions, queue, map, storage, log, state->cursor, state->slot_bar, state, assets);
		
		#if _DEBUG // NOTE(): Render the input directions on the map.
		v2s base_p = cursor_mode_active ? state->cursor->p : entity->p;
		for (s32 index = 0; index < 4; index++)
			RenderIsoTile(out, map, AddS(base_p, directions[index]), SetAlpha(Orange(), 0.5f), true, 0);
		#endif
		
		if (input_valid && (cursor_mode_active == false) && (queue->action_points > 0))
		{
	        if (MoveEntity(map, entity, directions[direction]))
	        {
	        	ApplyTileEffects(entity->p, state, entity);
				// NOTE(): Consume moves
				queue->action_points--;
#if ENABLE_DEBUG_PATHFINDING
				memory_t memory = {0};
				memory.size = ArraySize(state->debug_memory);
				memory._memory = state->debug_memory;
				ComputeDistances(state->map, entity->p.x, entity->p.y, memory);
#endif
			}
		}
}

fn void ResolveAsynchronousActionQueue(turn_queue_t *queue, entity_t *user, command_buffer_t *out, f32 dt, assets_t *assets, map_t *map)
{
	for (s32 index = 0; index < queue->action_count; index++)
	{
		async_action_t *action = &queue->actions[index];
		b32 Finished = false;

		if ((action->type == action_ranged_attack) ||
			(action->type == action_throw))
		{
			v2 target_p = GetTileCenter(map, action->target_p);
			f32 distance = Distance(user->deferred_p, target_p);
			f32 t = action->t / (distance * 0.005f);
			action->t += dt * 0.7f;
			DrawRangedAnimation(out, user->deferred_p, target_p, &assets->SlimeBall, t);
			Finished = (t >= 1.0f);
		}
		else
		{
			Finished = true;
		}

		if (Finished)
		{
			queue->actions[index--] = queue->actions[--queue->action_count];
			ActivateSlotAction(user, GetEntity(queue->storage, action->target_id),
				action->type, action->target_p, queue->storage, map);
		}
	}
}

fn v2 CameraTracking(v2 p, v2 player_world_pos, v2 viewport, f32 dt)
{
	v2 player_iso_pos = ScreenToIso(player_world_pos);
	
	v2 screen_center = Scale(Scale(viewport, 1.0f / (f32)VIEWPORT_INTEGER_SCALE), 0.5f);
	v2 camera_offset = Sub(screen_center, player_iso_pos);
	p = Lerp2(p, camera_offset, 5.0f * dt);
	return p;
}

fn void PushTurn(turn_queue_t *queue, entity_t *entity)
{
	if (queue->num < ArraySize(queue->entities))
		queue->entities[queue->num++] = entity->id;
}

fn void DefaultTurnOrder(turn_queue_t *queue, entity_storage_t *storage)
{
	s32 player_count = 0;
	entity_t *players[16] = {0};

	for (s32 index = 0; index < storage->num; index++)
	{
		entity_t *entity = &storage->entities[index];
		if (IsHostile(entity))
			PushTurn(queue, entity);
		else
			players[player_count++] = entity;
	}

	Assert(player_count < ArraySize(players));
	for (s32 index = 0; index < player_count; index++)
		PushTurn(queue, players[index]);
}

fn entity_t *NextInOrder(turn_queue_t *queue, entity_storage_t *storage)
{
	entity_t *result = 0;
	if (queue->num > 0)
	{
		result = GetEntity(storage, queue->entities[queue->num - 1]);
		if (!result)
			queue->num--; // NOTE(): The ID is invalid, pull it from the queue.
	}
	return result;
}

fn int32_t IsEntityActive(turn_queue_t *queue, entity_storage_t *storage, entity_id_t id)
{
	entity_t *result = NextInOrder(queue, storage);
	if (result)
		return (result->id == id);
	return 0;
}

fn entity_t *PeekNextTurn(turn_queue_t *queue, entity_storage_t *storage)
{
	entity_t *result = 0;
	if (queue->num >= 2)
		result = GetEntity(storage, queue->entities[queue->num - 2]);
	return result;
}

fn void AcceptTurn(turn_queue_t *queue, entity_t *entity)
{
	DebugAssert(queue->turn_inited == true); // NOTE(): Propably a bug?

	Assert(queue->num > 0);
	queue->num--;
	queue->turn_inited = false;
	queue->prev_turn_entity = entity->id;
	queue->seconds_elapsed = 0.0f;
}

fn void ConsumeActionPoints(turn_queue_t *queue, s32 count)
{
	queue->action_points--;
}

fn void TurnKernel(game_world_t *state, entity_storage_t *storage, map_t *map, turn_queue_t *queue, f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out, assets_t *assets)
{
	// NOTE(): Setup
	queue->storage = storage;
	SetGlobalOffset(out, state->camera_position); // NOTE(): Let's pass the camera position via the PushRenderOutput call instead of this SetGlobalOffset stuff.
	// NOTE(): Process the current turn

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
			queue->seconds_elapsed = 0.0f;

			#if ENABLE_TURN_SYSTEM_DEBUG_LOGS
			DebugLog("initiating turn for entity#%i", entity->id);
			#endif
			// NOTE(): Begin the turn.
		}

		Assert(queue->turn_inited);
		queue->seconds_elapsed += dt;
		// NOTE(): The turn will "stall" until AcceptTurn() is called.

		// NOTE(): DEBUG draw the "discrete" p.
		#if _DEBUG
		RenderIsoTile(out, map, entity->p, Red(), (queue->interp_state == interp_wait_for_input), 0);
		#endif

		// NOTE(): We're focusing the camera either on a cursor or on a player position,
		// depending on the current mode.
		queue->focus_p = state->cursor->active ? GetTileCenter(map, state->cursor->p) : entity->deferred_p;
		state->camera_position = CameraTracking(state->camera_position, queue->focus_p, GetViewport(input), dt);
		
		if (entity->flags & entity_flags_controllable) // NOTE(): The "player" code.
		{
			// TODO(): We should buffer inputs here in case
			// the player pressed a button while the
			// animation is still being played.
			if (queue->action_count == 0)
				ListenForUserInput(entity, state, storage, map, queue, dt, input, cons, log, out, assets);
			ResolveAsynchronousActionQueue(queue, entity, out, dt, assets, map);

			// NOTE(): End the turn if we run out of all action points.
			if ((queue->action_points <= 0) && (queue->action_count == 0))
				AcceptTurn(queue, entity);
		}
		else // NOTE(): AI
		{
			s32 range = ENEMY_DEBUG_RANGE;
			DrawHighlightArea(out, map, entity->p, range, Green()); // Range

			f32 speed_mul = TURN_SPEED_NORMAL;
			queue->time += dt * speed_mul;

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
					 		entity_id_t target = AttemptAttack(state, entity, range);
					 		if (target)
					 		{
								ChangeQueueState(queue, interp_attack);
								queue->attack_target = target;
							}
							else
							{
								ChangeQueueState(queue, interp_accept);
							}
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
						AcceptTurn(queue, entity);
						#ifdef ENABLE_TURN_SYSTEM_DEBUG_LOGS
						DebugLog("turn finished in %.2f seconds", queue->seconds_elapsed);
						#endif
					}
				} break;
			}
		}
	}

	GarbageCollect(queue);
	ControlPanel(state->turns, &cons, state->storage);
}