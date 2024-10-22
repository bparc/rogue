fn void PushTurn(turn_queue_t *queue, entity_t *entity)
{
	if (queue->num < ArraySize(queue->entities))
		queue->entities[queue->num++] = entity->id;
}

fn void DefaultTurnOrder(turn_queue_t *queue)
{
	entity_storage_t *storage = queue->storage;

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

fn entity_t *GetActiveUnit(const turn_queue_t *queue)
{
	entity_t *result = 0;
	if (queue->num > 0)
		result = GetEntity(queue->storage, queue->entities[queue->num - 1]);
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

fn s32 ConsumeActionPoints(turn_queue_t *queue, s32 count)
{
	s32 sufficient = queue->action_points - count > 0;
	if (sufficient)
	{
		DebugLog("Used %i action points", count);
		queue->action_points -= count;
	}
	else
	{
		DebugLog("Insufficient amount of action points! (%i req.)", count);
	}
	return sufficient;
}

fn void QueryAsynchronousAction(turn_queue_t *queue, action_type_t type, entity_t *target, v2s target_p)
{
	async_action_t *result = 0;
	if ((queue->action_count < ArraySize(queue->actions)))
	{
		result = &queue->actions[queue->action_count++];
		
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
		result->action_type.type = type;
	}
}

fn void ControlPanel(turn_queue_t *queue, const virtual_controls_t *cons, entity_storage_t *storage)
{
	if (WentDown(cons->debug[0])) // Toggle
		queue->break_mode_enabled = !queue->break_mode_enabled;
	if (WentDown(cons->debug[2])) // Toggle
		queue->god_mode_enabled = !queue->god_mode_enabled;

	DebugPrint(
		"ACT %i | BRK (F1): %s%s%s | GOD (F3): %s",
		(queue->action_points),
		(queue->break_mode_enabled ? "ON" : "OFF"),
		(queue->interp_state == interp_wait_for_input) ? " | STEP (F2) -> " : "",
		(queue->interp_state == interp_wait_for_input) ? interpolator_state_t_names[queue->requested_state] : "",
		(queue->god_mode_enabled ? "ON" : "OFF"))
	;

	if ((queue->interp_state == interp_wait_for_input) && WentDown(cons->debug[1]))
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

fn evicted_entity_t *GetEvictedEntity(turn_queue_t *queue, entity_id_t ID)
{
	for (s32 index = 0; index < queue->num_evicted_entities; index++)
	{
		evicted_entity_t *entity = &queue->evicted_entities[index];
		if (entity->id == ID)
			return entity;
	}
	return NULL;
}

fn void GarbageCollect(game_world_t *Game, turn_queue_t *queue, f32 dt)
{
	// TODO(): This will mess up the turn oder...
	entity_storage_t *storage = queue->storage;
	for (s32 index = 0; index < storage->num; index++)
	{
		entity_t *entity = &storage->entities[index];
		if (entity->flags & entity_flags_deleted)
		{
			Perish(Game, entity);

			if (queue->num_evicted_entities < ArraySize(queue->evicted_entities))
			{
				evicted_entity_t *evicted = &queue->evicted_entities[queue->num_evicted_entities++];
				evicted->entity = *entity;
				evicted->time_remaining = 1.0f;
			}

			storage->entities[index--] = storage->entities[--storage->num];
		}
	}

	for (s32 index = 0; index < queue->num_evicted_entities; index++)
	{
		evicted_entity_t *entity = &queue->evicted_entities[index];
		entity->time_remaining -= (dt * ENTITY_EVICTION_SPEED);
		if (entity->time_remaining <= 0.0f)
			queue->evicted_entities[index--] = queue->evicted_entities[--queue->num_evicted_entities];
	}
}

fn void inline ListenForUserInput(entity_t *entity, game_world_t *state,
	entity_storage_t *storage, map_t *map, turn_queue_t *queue,
	f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out, assets_t *assets)
{
		// NOTE(): Listen for the player input.
		const v2s *directions = cardinal_directions;
		#if ENABLE_DIAGONAL_MOVEMENT
		if (IsKeyPressed(input, key_code_shift))
			directions = diagonal_directions;
		#endif

		if (WentDown(cons.y) && queue->action_points > 0) {
			entity->has_hitchance_boost = true;

			queue->action_points = 0;
			DebugLog("Skipped turn with action points remaining. Hit chance increased by %d%% for next attack.", SKIP_TURN_HIT_CHANCE_INCREASE);
		}
		
		s32 direction = GetDirectionalInput(input);
		b32 input_valid = (direction >= 0) && (direction < 4);
		b32 cursor_mode_active = state->cursor->active; // NOTE(): The cursor_active flag needs to be stored *before* calling DoCursor. This is actually the correct order. For reasons.
		DoCursor(state, assets, log, out, cons, entity, input_valid, direction, directions);
		
		#if _DEBUG // NOTE(): Render the input directions on the map.
		v2s base_p = cursor_mode_active ? state->cursor->p : entity->p;
		for (s32 index = 0; index < 4; index++)
			RenderIsoTile(out, map, Add32(base_p, directions[index]), A(Orange(), 0.5f), true, 0);
		#endif
		
		if (input_valid && (cursor_mode_active == false) && (queue->action_points > 0))
		{
	        if (Move(state, entity, directions[direction]) && (queue->god_mode_enabled == false))
	        {
	        	ApplyTileEffects(entity->p, state, entity);
				// NOTE(): Consume moves
				queue->action_points--;
			}
		}
}

fn void ResolveAsynchronousActionQueue(turn_queue_t *queue, entity_t *user, command_buffer_t *out, f32 dt, assets_t *assets, game_world_t *state)
{
	for (s32 index = 0; index < queue->action_count; index++)
	{
		async_action_t *action = &queue->actions[index];
		action->action_type.params = DefineActionTypeParams(user, action->action_type);
		b32 Finished = false;

		if ((action->action_type.type == action_ranged_attack) ||
			(action->action_type.type == action_throw))
		{
			v2 target_p = GetTileCenter(state->map, action->target_p);
			f32 distance = Distance(user->deferred_p, target_p);
			f32 t = action->t / (distance * 0.005f);
			
			action->t += dt * 1.5f;

			DrawRangedAnimation(out, user->deferred_p, target_p, &assets->PlayerGrenade, t);
			
			Finished = (t >= 1.0f);
		}
		else
		{
			Finished = true;
		}

		if (Finished)
		{
			queue->actions[index--] = queue->actions[--queue->action_count];
			ActivateSlotAction(state, user, GetEntity(queue->storage, action->target_id), &action->action_type);
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

fn void DebugDrawPathSystem(turn_queue_t *queue, map_t *map, command_buffer_t *out)
{
	if (queue->break_mode_enabled)
	{
		path_t *path = &queue->path;
		for (s32 index = 0; index < path->length; index++)
		{
			path_tile_t *Tile = &path->tiles[index];
			v4 color = Orange();
			RenderIsoTile(out, map, Tile->p, color, true, 0);
		}
	}
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
		EstablishTurnOrder(state, queue);
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
			ResolveAsynchronousActionQueue(queue, entity, out, dt, assets, state);

			// NOTE(): End the turn if we run out of all action points.
			if ((queue->action_points <= 0) && (queue->action_count == 0))
				AcceptTurn(queue, entity);
		}
		else // NOTE(): AI
		{
			s32 range = ENEMY_DEBUG_RANGE;
			RenderRange(out, map, entity->p, range, Green()); // Range

			f32 speed_mul = TURN_SPEED_NORMAL;
			queue->time += dt * speed_mul;

			switch(queue->interp_state)
			{
			case interp_wait_for_input:
				{
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
						#ifdef ENABLE_TURN_SYSTEM_DEBUG_LOGS
						DebugLog("turn finished in %.2f seconds", queue->seconds_elapsed);
						#endif
						AcceptTurn(queue, entity);
					}
				} break;
			}
		}
	}

	GarbageCollect(state, queue, dt);
	DebugDrawPathSystem(queue, map, out);
}