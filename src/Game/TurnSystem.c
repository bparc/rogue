fn void PushTurn(turn_system_t *System, entity_t *entity)
{
	if ((System->QueueSize < ArraySize(System->Queue)) && entity)
		System->Queue[System->QueueSize++] = entity->id;
}

fn void ClearTurnQueue(turn_system_t *System)
{
	System->QueueSize = 0;;
}

fn entity_t *NextInOrder(turn_system_t *System, entity_storage_t *storage)
{
	entity_t *result = 0;
	if (System->QueueSize > 0)
	{
		result = GetEntity(storage, System->Queue[System->QueueSize - 1]);
		if (!result)
			System->QueueSize--; // NOTE(): The ID is invalid - pull it from the System.
	}
	return result;
}

fn entity_t *GetActiveUnit(const turn_system_t *System)
{
	entity_t *result = 0;
	if (System->QueueSize > 0)
		result = GetEntity(System->storage, System->Queue[System->QueueSize - 1]);
	return result;
}

fn b32 IsActionQueueCompleted(const turn_system_t *System)
{
	return (System->action_count == 0);
}

fn int32_t IsEntityActive(turn_system_t *System, entity_storage_t *storage, entity_id_t id)
{
	entity_t *result = NextInOrder(System, storage);
	if (result)
		return (result->id == id);
	return 0;
}

fn entity_t *PeekNextTurn(turn_system_t *System, entity_storage_t *storage)
{
	entity_t *result = 0;
	if (System->QueueSize >= 2)
		result = GetEntity(storage, System->Queue[System->QueueSize - 2]);
	return result;
}

fn void AcceptTurn(turn_system_t *System, entity_t *entity)
{
	DebugAssert(System->turn_inited == true); // NOTE(): Propably a bug?

	Assert(System->QueueSize > 0);
	System->QueueSize--;
	System->turn_inited = false;
	System->prev_turn_entity = entity->id;
	System->seconds_elapsed = 0.0f;
}

fn s32 ConsumeMovementPoints(turn_system_t *System, s32 count)
{
	System->movement_points -= count;
	return true;
}

fn s32 ConsumeActionPoints(turn_system_t *System, s32 count)
{
	s32 sufficient = System->action_points - count >= 0;
	if (sufficient)
		System->action_points -= count;
	else
		DebugLog("Insufficient amount of action points! (%i req.)", count);
	return sufficient;
}

fn void QueryAsynchronousAction(turn_system_t *System, action_type_t type, entity_id_t target, v2s target_p)
{
	async_action_t *result = 0;
	if ((System->action_count < ArraySize(System->actions)))
	{
		result = &System->actions[System->action_count++];
		
		ZeroStruct(result);
		result->target_id = target;
		result->target_p  = target_p;

		result->action_type.type = type;
	}
}

fn void ControlPanel(turn_system_t *System, const virtual_controls_t *cons, entity_storage_t *storage)
{
	if (WentDown(cons->debug[0])) // Toggle
		System->break_mode_enabled = !System->break_mode_enabled;
	if (WentDown(cons->debug[2])) // Toggle
		System->god_mode_enabled = !System->god_mode_enabled;
	if (WentDown(cons->debug[3])) // Toggle
		System->free_camera_mode_enabled = !System->free_camera_mode_enabled;

	DebugPrint(
		"ACT %i | MOV %i | BRK (F1): %s%s%s | GOD (F3): %s | FREE (F4): %s",
		(System->action_points),
		(System->movement_points),
		(System->break_mode_enabled ? "ON" : "OFF"),
		(System->interp_state == interp_wait_for_input) ? " | STEP (F2) -> " : "",
		(System->interp_state == interp_wait_for_input) ? interpolator_state_t_names[System->requested_state] : "",
		(System->god_mode_enabled ? "ON" : "OFF"),
		(System->free_camera_mode_enabled ? "ON" : "OFF"));
}

fn inline s32 ChangeQueueState(turn_system_t *System, interpolator_state_t state)
{
	s32 result = false;
	System->time = 0.0f;

	#if _DEBUG
	result = (System->break_mode_enabled == false);
	if (result)
	{
		System->interp_state = state;
	}
	else
	{
		System->interp_state = interp_wait_for_input;
		System->requested_state = state;
		System->request_step = false;
	}
	#else
	System->interp_state = state;
	result = true;
	#endif
	return result;
}

fn evicted_entity_t *GetEvictedEntity(turn_system_t *System, entity_id_t ID)
{
	for (s32 index = 0; index < System->num_evicted_entities; index++)
	{
		evicted_entity_t *entity = &System->evicted_entities[index];
		if (entity->id == ID)
			return entity;
	}
	return NULL;
}

fn void GarbageCollect(game_state_t *Game, turn_system_t *System, f32 dt)
{
	// TODO(): This will mess up the turn oder...
	entity_storage_t *storage = System->storage;
	for (s32 index = 0; index < storage->EntityCount; index++)
	{
		entity_t *entity = &storage->entities[index];
		if (entity->flags & entity_flags_deleted)
		{
			Perish(Game, entity);

			if (System->num_evicted_entities < ArraySize(System->evicted_entities))
			{
				evicted_entity_t *evicted = &System->evicted_entities[System->num_evicted_entities++];
				evicted->entity = *entity;
				evicted->time_remaining = 1.0f;
			}

			storage->entities[index--] = storage->entities[--storage->EntityCount];
		}
	}

	for (s32 index = 0; index < System->num_evicted_entities; index++)
	{
		evicted_entity_t *entity = &System->evicted_entities[index];
		entity->time_remaining -= (dt * ENTITY_EVICTION_SPEED);
		if (entity->time_remaining <= 0.0f)
			System->evicted_entities[index--] = System->evicted_entities[--System->num_evicted_entities];
	}
}

fn void Brace(turn_system_t *System, entity_t *entity)
{
	if (System->movement_points >= 2 && ((entity->hitchance_boost_multiplier + 0.1f) <= 2.0f))
	{
		entity->has_hitchance_boost = true;
		System->movement_points -= 2;
		entity->hitchance_boost_multiplier += 0.1f;
		DebugLog("Used 2 movement points to brace for a hit chance bonus. Hit chance multiplied by %g for next attack.", entity->hitchance_boost_multiplier);
	}
	else if (System->movement_points >= 2 && entity->hitchance_boost_multiplier + 0.1f > 2.0f)
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

fn void ResolveAsynchronousActionQueue(turn_system_t *System, entity_t *user, command_buffer_t *out, f32 dt, assets_t *assets, game_state_t *state)
{
	const map_t *map = state->map;

	for (s32 index = 0; index < System->action_count; index++)
	{
		async_action_t *action = &System->actions[index];
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
			RenderRangedAnimation(out, To, From, params->animation_ranged, time);
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
			System->actions[index--] = System->actions[--System->action_count];
			CommitAction(state, user, GetEntity(System->storage, action->target_id), &action->action_type, action->target_p);
		}

		action->elapsed += dt * 1.0f;
	}
}

fn void AI(game_state_t *state, entity_storage_t *storage, map_t *map, turn_system_t *System, f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out, assets_t *assets, entity_t *entity)
{
	RenderRange(out, map, entity->p, ENEMY_DEBUG_RANGE, Green());

	switch(System->interp_state)
	{
	case interp_wait_for_input:
		{
			if (WentDown(cons.debug[1]))
			{
				System->interp_state = System->requested_state;
				System->time = 0.0f;
			}
		} break;
	Request:
	case interp_request:
		{
			System->starting_p = entity->p;
			s32 cost = Decide(state, entity);
				System->action_points -= cost;
			
			if (!ChangeQueueState(System, interp_transit))
				break;
			System->time = dt;
		}; // NOTE(): Intentional fall-through. We want to start handling transit immediatly, instead of waiting an addidional frame.
	case interp_transit:
		{
			v2 From = GetTileCenter(map, System->starting_p);
			v2 To = GetTileCenter(map, entity->p);
			entity->deferred_p = Lerp2(From, To, System->time);

			if ((System->time >= 1.0f))
			{
			 	if (System->action_points > 0)
			 	{
			 		if (ChangeQueueState(System, interp_request))
			 			goto Request;
			 	}
			 	else
			 	{
			 		ScheduleEnemyAction(state, entity, ENEMY_DEBUG_RANGE);

			 		if ((IsActionQueueCompleted(System) == false))
						ChangeQueueState(System, interp_action);
					else
						ChangeQueueState(System, interp_accept);
			 	}
			}
		} break;
	case interp_action:
		{
			// NOTE(): Stall until all action are completed.
			if ((IsActionQueueCompleted(System) == false))
				System->time = 0.0f;

			if (System->time >= 3.0f)
				ChangeQueueState(System, interp_accept);
		} break;
	case interp_accept:
		{
			if (System->time > 0.1f)
			{
				#if ENABLE_TURN_SYSTEM_DEBUG_LOGS
				DebugLog("turn finished in %.2f seconds", System->seconds_elapsed);
				#endif
				AcceptTurn(System, entity);
			}
		} break;
	}	
}

fn void BeginTurnSystem(turn_system_t *Queue, game_state_t *Game, f32 dt)
{
	Queue->Player = Game->Player;
	Queue->map = Game->map;
	Queue->storage = Game->storage;
	Queue->seconds_elapsed += dt;
	Queue->time += (dt * 4.0f);
}

fn void EndTurnSystem(turn_system_t *Queue, game_state_t *Game)
{
	
}

fn void InteruptTurn(turn_system_t *Queue, entity_t *Entity)
{
	// NOTE(): Alert/Interupt Player Turn
	PushTurn(Queue, Entity);
	Queue->turn_inited = false;
}

fn inline s32 CheckTurnInterupts(game_state_t *state, entity_t *ActiveEntity)
{
	turn_system_t *System = state->turns;
	s32 Interupted = false;

	entity_storage_t *storage = System->storage;
	for (s32 Index = 0; Index < storage->EntityCount; Index++)
	{
		entity_t *Entity = &storage->entities[Index];
		if (IsHostile(Entity) && (!Entity->Alerted))
		{
			if (IsLineOfSight(state->map, Entity->p, ActiveEntity->p))
			{
				CreateCombatText(state->particles, Entity->deferred_p, combat_text_alerted);
				Entity->Alerted = true;

				InteruptTurn(System, Entity);
				Interupted = true;
				break;
			}
		}
	}

	return (Interupted);
}

fn b32 IsCellEmpty(turn_system_t *System, v2s p)
{
	entity_t *collidingEntity = GetEntityByPosition(System->storage, p);

	if (collidingEntity == NULL)
    {
		return IsTraversable(System->map, p);
	}

	return false;
}

fn b32 Move(turn_system_t *System, entity_t *entity, v2s offset)
{
	v2s requested_p = Add32(entity->p, offset);
	b32 valid = IsCellEmpty(System, requested_p);
	if (valid)
		entity->p = requested_p;
	return (valid);
}

fn int MoveFitsWithSize(turn_system_t* System, entity_t *requestee, v2s requestedPos)
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
        if (!IsCellEmpty(System, moveCoords[i])) {
			return false;
        }
    }

    return true;
}

fn b32 Launch(turn_system_t *System, v2s source, entity_t *target, u8 push_distance, s32 strength)
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