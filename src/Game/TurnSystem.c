fn void PushTurn(turn_queue_t *queue, entity_t *entity)
{
	if (queue->num < ArraySize(queue->entities))
		queue->entities[queue->num++] = entity->id;
}

fn void ClearTurnQueue(turn_queue_t *queue)
{
	queue->num = 0;;
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

fn void GarbageCollect(game_state_t *Game, turn_queue_t *queue, f32 dt)
{
	// TODO(): This will mess up the turn oder...
	entity_storage_t *storage = queue->storage;
	for (s32 index = 0; index < storage->EntityCount; index++)
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

			storage->entities[index--] = storage->entities[--storage->EntityCount];
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

fn void DoDamage(game_state_t *game, entity_t *user, entity_t *target, s32 damage, const char *damage_type_prefix)
{
    damage = damage + (rand() % 3);
    LogLn(game->log, "%shit! inflicted %i %s of %sdamage upon the target!",
        damage_type_prefix, damage, damage == 1 ? "point" : "points", damage_type_prefix);
    TakeHP(target, (s16)damage);
    CreateDamageNumber(game->particles, Add(target->deferred_p, V2(-25.0f, -25.0f)), damage);

    if ((rand() / (float)RAND_MAX) < 20) {
        blood_type_t blood_type = (target->enemy_type == 0) ? blood_red : blood_green;
        hit_velocity_t hit_velocity = (rand() % 2 == 0) ? high_velocity : low_velocity; // Just a temporary thing until we add ammo types
        BloodSplatter(game->map, user->p, target->p, blood_type, hit_velocity);
    }
}

fn inline void AOE(game_state_t *state, entity_t *user, entity_t *target, const action_params_t *params)
{
    entity_storage_t *storage = state->storage;
    
    const char *prefix = "blast ";
    s32 damage = params->damage;
    v2s area = params->area;
    s32 radius_inner = area.x;
    s32 radius_outer = area.x * (s32)2;
    v2s explosion_center = state->cursor->p;
    for (s32 i = 0; i < storage->EntityCount; i++)
    {
        entity_t *entity = &storage->entities[i];
        f32 distance = DistanceV2S(explosion_center, entity->p);
        
        if (distance <= radius_inner) {
            DoDamage(state, user, entity, damage, prefix);
        } else if (distance <= radius_outer) {
            DoDamage(state, user, entity, damage, prefix);
            Launch(state->turns, explosion_center, entity, 2, 25);
        }
    }
}

fn inline void SingleTarget(game_state_t *state, entity_t *user, entity_t *target, const action_params_t *params)
{
    if ((IsPlayer(user) && state->turns->god_mode_enabled))
    {
        DoDamage(state, user, target, target->health, "");
    }
    else
    {
        s32 chance = CalculateHitChance(user, target, params->type);
        s32 roll = rand() % 100;
        s32 roll_crit = rand() % 100;

        b32 missed = !(roll < chance);
        b32 crited = (roll_crit < CRITICAL_DAMAGE_MULTIPLIER);
        b32 grazed = ((roll >= chance)) && (roll < (chance + GRAZE_THRESHOLD));

        if ((missed == false))
        {    
            if (crited) {
                DoDamage(state, user, target, params->damage * CRITICAL_DAMAGE_MULTIPLIER, "critical ");
                CreateCombatText(state->particles, target->deferred_p, 0);
            } else {
                DoDamage(state, user, target, params->damage, "");
                CreateCombatText(state->particles, target->deferred_p, 1);
            }
        }
        else
        {
            if (grazed) {
                DoDamage(state, user, target, params->damage / 2, "graze ");
                CreateCombatText(state->particles, target->deferred_p, 3);
            } else {
                LogLn(state->log, "missed!");
                CreateDamageNumber(state->particles, target->deferred_p, 0);
                CreateCombatText(state->particles, target->deferred_p, 2);
            }
        }
    }
}

fn void CommitAction(game_state_t *state, entity_t *user, entity_t *target, action_t *action, v2s target_p)
{
    const action_params_t *params = GetParameters(action->type);

    switch (params->mode)
    {
    case action_mode_damage:
    {
        if (IsZero(params->area))
        {
            SingleTarget(state, user, target, params);
        }
        else
        {
            AOE(state, user, target, params);
        }

        // NOTE(): Reset per-turn attack buffs/modifiers.
        user->has_hitchance_boost = false;
        user->hitchance_boost_multiplier = 1.0f;
    } break;
    case action_mode_heal: Heal(target, (s16) params->value); break;
    case action_mode_dash: user->p = target_p; break;
    }
}

fn void ResolveAsynchronousActionQueue(turn_queue_t *queue, entity_t *user, command_buffer_t *out, f32 dt, assets_t *assets, game_state_t *state)
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

fn void AI(game_state_t *state, entity_storage_t *storage, map_t *map, turn_queue_t *queue, f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out, assets_t *assets, entity_t *entity)
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

fn void Camera(game_state_t *Game, entity_t *TrackedEntity, const client_input_t *Input, f32 dt)
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

fn void TurnQueueBeginFrame(turn_queue_t *Queue, game_state_t *Game, f32 dt)
{
	Queue->map = Game->map;
	Queue->storage = Game->storage;
	Queue->seconds_elapsed += dt;
	Queue->time += (dt * 4.0f);
}

fn void TurnQueueEndFrame(turn_queue_t *Queue, game_state_t *Game)
{

}

fn void InteruptTurn(turn_queue_t *Queue, entity_t *Entity)
{
	// NOTE(): Alert/Interupt Player Turn
	PushTurn(Queue, Entity);
	Queue->turn_inited = false;
}

fn inline s32 CheckTurnInterupts(game_state_t *state, entity_t *ActiveEntity)
{
	turn_queue_t *queue = state->turns;
	s32 Interupted = false;

	entity_storage_t *storage = queue->storage;
	for (s32 Index = 0; Index < storage->EntityCount; Index++)
	{
		entity_t *Entity = &storage->entities[Index];
		if (IsHostile(Entity) && (!Entity->Alerted))
		{
			if (IsLineOfSight(state->map, Entity->p, ActiveEntity->p))
			{
				CreateCombatText(state->particles, Entity->deferred_p, combat_text_alerted);
				Entity->Alerted = true;

				InteruptTurn(queue, Entity);
				Interupted = true;
				break;
			}
		}
	}

	return (Interupted);
}

fn b32 IsWorldPointEmpty(turn_queue_t *System, v2s p)
{
	entity_t *collidingEntity = GetEntityByPosition(System->storage, p);

	if (collidingEntity == NULL)
    {
		return IsTraversable(System->map, p);
	}

	return false;
}

fn b32 Move(turn_queue_t *System, entity_t *entity, v2s offset)
{
	v2s requested_p = Add32(entity->p, offset);
	b32 valid = IsWorldPointEmpty(System, requested_p);
	if (valid)
		entity->p = requested_p;
	return (valid);
}

fn int MoveFitsWithSize(turn_queue_t* System, entity_t *requestee, v2s requestedPos)
{
    int currentX = requestee->p.x;
    int currentY = requestee->p.y;

    int deltaX = requestedPos.x - currentX; // For example: if requestee is at 8,8 and requestedPos is at 9,9
    int deltaY = requestedPos.y - currentY; //				then delta x will be 9-8=1

    v2s moveCoords[3]; // For diagonal movement there's three coords
    int numCoords = 0;

    // Moving up
    if (deltaX == 0 && deltaY == -1) {
        moveCoords[0] = (v2s){currentX, currentY - 1};     // Tile above (1,1)
        moveCoords[1] = (v2s){currentX + 1, currentY - 1}; // Tile above (2,1)
        numCoords = 2;
    }
    // Moving down
    else if (deltaX == 0 && deltaY == 1) {
        moveCoords[0] = (v2s){currentX, currentY + 2};     // Tile below (1,2)
        moveCoords[1] = (v2s){currentX + 1, currentY + 2}; // Tile below (2,2)
        numCoords = 2;
    }
    // Moving left
    else if (deltaX == -1 && deltaY == 0) {
        moveCoords[0] = (v2s){currentX - 1, currentY};     // Tile to the left (1,1)
        moveCoords[1] = (v2s){currentX - 1, currentY + 1}; // Tile to the left (1,2)
        numCoords = 2;
    }
    // Moving right
    else if (deltaX == 1 && deltaY == 0) {
        moveCoords[0] = (v2s){currentX + 2, currentY};     // Tile to the right (2,1)
        moveCoords[1] = (v2s){currentX + 2, currentY + 1}; // Tile to the right (2,2)
        numCoords = 2;
    }
    // Moving up-left (diagonal)
    else if (deltaX == -1 && deltaY == -1) {
        moveCoords[0] = (v2s){currentX - 1, currentY - 1}; // Top-left
        moveCoords[1] = (v2s){currentX, currentY - 1};     // Top center
        moveCoords[2] = (v2s){currentX - 1, currentY};     // Left center
        numCoords = 3;
    }
    // Moving up-right (diagonal)
    else if (deltaX == 1 && deltaY == -1) {
        moveCoords[0] = (v2s){currentX + 2, currentY - 1}; // Top-right
        moveCoords[1] = (v2s){currentX + 1, currentY - 1}; // Top center
        moveCoords[2] = (v2s){currentX + 2, currentY};     // Right center
        numCoords = 3;
    }
    // Moving down-left (diagonal)
    else if (deltaX == -1 && deltaY == 1) {
        moveCoords[0] = (v2s){currentX - 1, currentY + 2}; // Bottom-left
        moveCoords[1] = (v2s){currentX, currentY + 2};     // Bottom center
        moveCoords[2] = (v2s){currentX - 1, currentY + 1}; // Left center
        numCoords = 3;
    }
    // Moving down-right (diagonal)
    else if (deltaX == 1 && deltaY == 1) {
        moveCoords[0] = (v2s){currentX + 2, currentY + 2}; // Bottom-right
        moveCoords[1] = (v2s){currentX + 1, currentY + 2}; // Bottom center
        moveCoords[2] = (v2s){currentX + 2, currentY + 1}; // Right center
        numCoords = 3;
    } else {
        //DebugLog("Invalid movement of entity id: %d");
        return false;
    }

    for (int i = 0; i < numCoords; i++) {
        if (!IsWorldPointEmpty(System, moveCoords[i])) {
			return false;
        }
    }

    return true;
}

fn b32 Launch(turn_queue_t *System, v2s source, entity_t *target, u8 push_distance, s32 strength)
{
    v2s direction = Sub32(target->p, source);

    if (direction.x != 0) direction.x = (direction.x > 0) ? 1 : -1;
    if (direction.y != 0) direction.y = (direction.y > 0) ? 1 : -1;

    s32 damage_per_tile = strength / (target->size.x * target->size.y);
    s32 total_damage = 0;

    for (s32 i = 0; i < push_distance; ++i) { // todo: make strength affect push_distance somewhat
        v2s next_pos = Add32(target->p, direction); // Slime is moved through each tile on the way

        if (!MoveFitsWithSize(System, target, next_pos)) {
            TakeHP(target, (s16)damage_per_tile);
            break;
        } else {
            target->p = next_pos;
            ApplyTileEffects(System->map, target);
        }
    }

    return false;
}