fn void PushTurn(turn_queue_t *queue, entity_t *entity)
{
	if (queue->num < ArraySize(queue->entities))
		queue->entities[queue->num++] = entity->id;
}

fn void ClearTurnQueue(turn_queue_t *queue)
{
	queue->num = 0;;
}

fn void DefaultTurnOrder(turn_queue_t *queue)
{
	ClearTurnQueue(queue);

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
			queue->num--; // NOTE(): The ID is invalid - pull it from the queue.
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

fn b32 IsActionQueueCompleted(const turn_queue_t *queue)
{
	return (queue->action_count == 0);
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

fn s32 ConsumeMovementPoints(turn_queue_t *queue, s32 count)
{
	queue->movement_points -= count;
	return true;
}

fn s32 ConsumeActionPoints(turn_queue_t *queue, s32 count)
{
	s32 sufficient = queue->action_points - count >= 0;
	if (sufficient)
		queue->action_points -= count;
	else
		DebugLog("Insufficient amount of action points! (%i req.)", count);
	return sufficient;
}

fn s32 CountHostiles(turn_queue_t *Queue)
{
	s32 Result = 0;
	for (s32 Index = 0; Index < Queue->num; Index++)
	{
		entity_t *Entity = GetEntity(Queue->storage, Queue->entities[Index]);
		if (IsHostile(Entity))
			Result++;
	}
	return Result;
}

fn void QueryAsynchronousAction(turn_queue_t *queue, action_type_t type, entity_id_t target, v2s target_p)
{
	async_action_t *result = 0;
	if ((queue->action_count < ArraySize(queue->actions)))
	{
		result = &queue->actions[queue->action_count++];
		
		ZeroStruct(result);
		result->target_id = target;
		result->target_p  = target_p;

		result->action_type.type = type;
	}
}

fn void ControlPanel(turn_queue_t *queue, const virtual_controls_t *cons, entity_storage_t *storage)
{
	if (WentDown(cons->debug[0])) // Toggle
		queue->break_mode_enabled = !queue->break_mode_enabled;
	if (WentDown(cons->debug[2])) // Toggle
		queue->god_mode_enabled = !queue->god_mode_enabled;
	if (WentDown(cons->debug[3])) // Toggle
		queue->free_camera_mode_enabled = !queue->free_camera_mode_enabled;

	DebugPrint(
		"ACT %i | MOV %i | BRK (F1): %s%s%s | GOD (F3): %s | FREE (F4): %s",
		(queue->action_points),
		(queue->movement_points),
		(queue->break_mode_enabled ? "ON" : "OFF"),
		(queue->interp_state == interp_wait_for_input) ? " | STEP (F2) -> " : "",
		(queue->interp_state == interp_wait_for_input) ? interpolator_state_t_names[queue->requested_state] : "",
		(queue->god_mode_enabled ? "ON" : "OFF"),
		(queue->free_camera_mode_enabled ? "ON" : "OFF"));
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

fn void Brace(turn_queue_t *queue, entity_t *entity)
{
	if (queue->movement_points >= 2 && ((entity->hitchance_boost_multiplier + 0.1f) <= 2.0f))
	{
		entity->has_hitchance_boost = true;
		queue->movement_points -= 2;
		entity->hitchance_boost_multiplier += 0.1f;
		DebugLog("Used 2 movement points to brace for a hit chance bonus. Hit chance multiplied by %g for next attack.", entity->hitchance_boost_multiplier);
	}
	else if (queue->movement_points >= 2 && entity->hitchance_boost_multiplier + 0.1f > 2.0f)
	{
		DebugLog("Brace invalid. Your hit chance multiplier for next attack is %g which is maximum.", entity->hitchance_boost_multiplier);
	}
}

const f32 ScaleTByDistance(v2 a, v2 b, f32 t)
{
	f32 result = t / (Distance(a, b) * 0.005f);
	return result;
}

fn void ResolveAsynchronousActionQueue(turn_queue_t *queue, entity_t *user, command_buffer_t *out, f32 dt, assets_t *assets, game_world_t *state)
{
	const map_t *map = state->map;

	for (s32 index = 0; index < queue->action_count; index++)
	{
		async_action_t *action = &queue->actions[index];
		const action_params_t *params = GetParameters(action->action_type.type);

		b32 finished = true;

		if (params->animation_ranged)
		{
			// NOTE(): Ranged actions take some amount of time
			// to finish.

			// NOTE(): The duration is proportional to the distance.

			v2 From = GetTileCenter(map, action->target_p);
			v2 To = user->deferred_p;
			f32 time = ScaleTByDistance(From, To, action->elapsed);
			finished = (time >= 1.0f);
			DrawRangedAnimation(out, To, From, params->animation_ranged, time);
		}
		else
		if (params->flags & action_display_move_name)
		{
			f32 time = action->elapsed;
			v2 X = EaseInThenOut(40.0f, 60.0f, 15.0f, time);
			f32 alpha = 1.0f - X.y;
			DrawDiegeticText(state, user->deferred_p, V2((-20.0f + X.x), -80.0f), A(White(), alpha), params->name);
			finished =  (time >= 1.0f);
		}

		if (finished)
		{
			queue->actions[index--] = queue->actions[--queue->action_count];
			CommitAction(state, user, GetEntity(queue->storage, action->target_id), &action->action_type, action->target_p);
		}

		action->elapsed += dt * 1.0f;
	}
}

fn v2 CameraTracking(v2 p, v2 player_world_pos, v2 viewport, f32 dt)
{
	v2 player_iso_pos = ScreenToIso(player_world_pos);
	
	v2 screen_center = Scale(viewport, 0.5f);
	v2 camera_offset = Sub(screen_center, player_iso_pos);
	p = Lerp2(p, camera_offset, 5.0f * dt);
	return p;
}

fn void AI(game_world_t *state, entity_storage_t *storage, map_t *map, turn_queue_t *queue, f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out, assets_t *assets, entity_t *entity)
{
	RenderRange(out, map, entity->p, ENEMY_DEBUG_RANGE, Green());

	switch(queue->interp_state)
	{
	case interp_wait_for_input:
		{
			if (WentDown(cons.debug[1]))
			{
				queue->interp_state = queue->requested_state;
				queue->time = 0.0f;
			}
		} break;
	Request:
	case interp_request:
		{
			queue->starting_p = entity->p;
			s32 cost = Decide(state, entity);
				queue->action_points -= cost;
			
			if (!ChangeQueueState(queue, interp_transit))
				break;
			queue->time = dt;
		}; // NOTE(): Intentional fall-through. We want to start handling transit immediatly, instead of waiting an addidional frame.
	case interp_transit:
		{
			v2 From = GetTileCenter(map, queue->starting_p);
			v2 To = GetTileCenter(map, entity->p);
			entity->deferred_p = Lerp2(From, To, queue->time);

			if ((queue->time >= 1.0f))
			{
			 	if (queue->action_points > 0)
			 	{
			 		if (ChangeQueueState(queue, interp_request))
			 			goto Request;
			 	}
			 	else
			 	{
			 		ScheduleEnemyAction(state, entity, ENEMY_DEBUG_RANGE);

			 		if ((IsActionQueueCompleted(queue) == false))
						ChangeQueueState(queue, interp_action);
					else
						ChangeQueueState(queue, interp_accept);
			 	}
			}
		} break;
	case interp_action:
		{
			// NOTE(): Stall until all action are completed.
			if ((IsActionQueueCompleted(queue) == false))
				queue->time = 0.0f;

			if (queue->time >= 3.0f)
				ChangeQueueState(queue, interp_accept);
		} break;
	case interp_accept:
		{
			if (queue->time > 0.1f)
			{
				#if ENABLE_TURN_SYSTEM_DEBUG_LOGS
				DebugLog("turn finished in %.2f seconds", queue->seconds_elapsed);
				#endif
				AcceptTurn(queue, entity);
			}
		} break;
	}	
}

fn void Camera(game_world_t *Game, entity_t *TrackedEntity, const client_input_t *Input, f32 dt)
{
	camera_t *Camera = Game->camera;
	turn_queue_t *queue = Game->turns;
	map_t *map = Game->map;

	// NOTE(): We're focusing the camera either on a cursor or on a player position,
	// depending on the current mode.
	v2 focus_p = Game->cursor->active ? GetTileCenter(map, Game->cursor->p) : TrackedEntity->deferred_p;
	if (!queue->free_camera_mode_enabled)
	{
		Game->camera->p = CameraTracking(Game->camera->p, focus_p, GetViewport(Input), dt);

		f32 delta = 0.0f;
		if (Input->wheel)
			delta = Input->wheel > 0 ? 1.0f : -1.0f;
		delta *= 0.2f;

		Game->camera->zoom += delta;
		if (Game->camera->zoom <= 0.0f)
			Game->camera->zoom = 0.0f;
	}
}

fn void TurnQueueBeginFrame(turn_queue_t *Queue, game_world_t *Game, f32 dt)
{
	Queue->storage = Game->storage;
	Queue->seconds_elapsed += dt;
	Queue->time += (dt * 4.0f);
}

fn void TurnSystem(game_world_t *state, entity_storage_t *storage, map_t *map, turn_queue_t *queue, f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out, assets_t *assets)
{
	entity_t *ActiveEnt = NULL;

	TurnQueueBeginFrame(queue, state, dt);
	ActiveEnt = NextInOrder(queue, storage);

	b32 TurnHasEnded = (ActiveEnt == NULL);
	if (TurnHasEnded)
		EstablishTurnOrder(state, queue);

	if (ActiveEnt)
	{
		if ((queue->turn_inited == false))
		{
			CloseCursor(state->cursor);

			s32 MovementPointCount = BeginTurn(state, ActiveEnt);
			SetupTurn(queue, MovementPointCount);
			
			Assert(queue->turn_inited);
		}

		Camera(state, ActiveEnt, input, dt);

		if (ActiveEnt->flags & entity_flags_controllable)
		{
			// NOTE(): Room
			room_t *CurrentRoom = RoomFromPosition(state->layout, ActiveEnt->p);
			b32 ChangedRooms = CurrentRoom->Index != queue->CurrentRoomIndex;
			queue->CurrentRoomIndex = CurrentRoom->Index;

			s32 HostileCount = CountHostiles(queue);
			if ((HostileCount == 0))
			{
				if (!queue->EncounterInited)
				{
					if (ChangedRooms)
					{
						v2s At = CurrentRoom->min;
						CreateSlime(state, Add32(At, V2S(7, 8)));
						CreateSlime(state, Add32(At, V2S(14, 5)));
						DefaultTurnOrder(queue);
						queue->EncounterInited = true;
					}
				}
				else
				{
					queue->EncounterInited = false;
					OpenEveryDoor(state->map, CurrentRoom);	
				}
			}

			// NOTE(): Controls
			dir_input_t DirInput = GetDirectionalInput(input);
			b32 CursorEnabled = IsCursorEnabled(state->cursor);
			DoCursor(state, out, cons, ActiveEnt, DirInput);
			
			if (WentDown(cons.Inventory))
				ToggleInventory(state->interface);

			// NOTE(): Move
			b32 AllowedToMove = IsActionQueueCompleted(queue) /* Can't move when skill animations are playing! */ &&
				(CursorEnabled == false) && (queue->movement_points > 0);
			if (DirInput.Inputed && AllowedToMove)
			{
				b32 Moved = Move(state, ActiveEnt, DirInput.Direction);
				if (Moved && (queue->god_mode_enabled == false))
				{
					ConsumeMovementPoints(queue, 1);
					ApplyTileEffects(state->map, ActiveEnt);
				}
			}

			// NOTE(): Finish
			b32 CantDoAnyAction = (queue->movement_points <= 0 && queue->action_points == 0);
			b32 TurnForcefullySkipped = WentDown(cons.EndTurn);
			b32 EndTurn = TurnForcefullySkipped || CantDoAnyAction;
			if (EndTurn)
			{
				if (TurnForcefullySkipped)
					Brace(queue, ActiveEnt);

				AcceptTurn(queue, ActiveEnt);
			}
		}
		else
		{
			AI(state, storage, map, queue, dt, input, cons, log, out, assets, ActiveEnt);
		}
	}

	ResolveAsynchronousActionQueue(queue, ActiveEnt, out, dt, assets, state);
	GarbageCollect(state, queue, dt);
	ControlPanel(state->turns, &cons, state->storage);
}