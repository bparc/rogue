// TODO(): It could be usefull
// to have some kind of "step-by-step"/"turn-by-turn"
// debugging feature.

fn void TurnKernel(game_world_t *state, entity_storage_t *storage, map_t *map, turn_queue_t *turns, f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out)
{
	SetGlobalOffset(out, state->camera_position); // NOTE(): Let's pass the camera position via the PushRenderOutput call instead of this SetGlobalOffset stuff.
	// NOTE(): Process the current turn

	// TODO(Arc): IMPORTANT! Invalidated IDs aren't handled properly by
	// the queue! This has to be fixed when we add the ability to
	// remove an entity from the storage.

	entity_t *entity = NextInOrder(turns, storage);
	if (entity == 0)
	{
		// NOTE(): We run out of the turns, time to schedule new ones.
		EstablishTurnOrder(state, turns, storage);
	}
	if (entity)
	{
		if ((turns->turn_inited == false))
		{
			turns->action_points = (BeginTurn(state, entity) + 1);
			turns->turn_inited = true;

			turns->interp_state = interp_request;
			turns->time = 0.0f;
			turns->time_elapsed = 0.0f;

			#if ENABLE_TURN_SYSTEM_DEBUG_LOGS
			DebugLog("initiating turn for entity#%i", entity->id);
			#endif
			// NOTE(): Begin the turn.
		}

		Assert(turns->turn_inited);
		// NOTE(): The turn will "stall" until AcceptTurn() is called.

		// NOTE(): DEBUG draw the entity's"discrete" p.
		#if _DEBUG
		RenderIsoTile(out, map, entity->p, Red(), false, 0);
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
				direction, considered_dirs, turns, map, storage, log, state->cursor);
			
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
				if(!IsOutOfBounds(state, peekPos)
					&& !IsWall(state, peekPos)
					&& !IsOtherEntity(state, peekPos))
				{
					if (turns->action_points > 0) {
			            MoveEntity(map, entity, considered_dirs[direction]);
			            ApplyTileEffects(peekPos, state, entity);
					}

					// NOTE(): Consume moves
					turns->action_points--;
					if (turns->action_points <= 0)
						AcceptTurn(turns);
				}
			}
		}
		else
		{
			// NOTE(): Animator
			f32 speed_mul = TURN_SPEED_NORMAL;
			turns->time += dt * speed_mul;
			turns->time_elapsed += dt;

			switch(turns->interp_state)
			{
			case interp_request:
				{
					// TODO(): Move the player code somewhere around here maybe?
					// It could potentially be a better way to structure this.
					turns->starting_p = entity->p;

					s32 cost = Decide(state, entity);
						turns->action_points -= cost;
					
					turns->interp_state = interp_transit;
					turns->time = 0.0f;
				} break;
			case interp_transit:
				{
					v2 a = GetTileCenter(map, turns->starting_p);
					v2 b = GetTileCenter(map, entity->p);
					entity->deferred_p = Lerp2(a, b, turns->time);
					if ((turns->time >= 1.0f))
					 {
					 	if (turns->action_points > 0)
					 	{
					 		turns->interp_state = interp_request;
					 	}
					 	else
					 	{
					 		s32 attempt = AttemptAttack(state, entity);
							turns->interp_state = attempt ? interp_attack : interp_accept;	
					 	}
						
						turns->time = 0.0f;
					}
				} break;
			case interp_attack:
				{
					if ((turns->time >= 0.5f))
					{
						turns->interp_state = interp_accept;
						turns->time = 0.0f;
					}
				} break;
			case interp_accept:
				{
					if (turns->time > 0.1f)
					{
						AcceptTurn(turns);

						#ifdef ENABLE_TURN_SYSTEM_DEBUG_LOGS
						DebugLog("TurnKernel(): turn finished in %.2f seconds", turns->time_elapsed);
						#endif
					}
				} break;
			}
		}
	}
}