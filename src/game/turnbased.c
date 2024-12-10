fn void PushTurn(turn_system_t *System, entity_t *entity)
{
	if ((System->QueueSize < ArraySize(System->Queue)) && entity)
	{
		System->Queue[System->QueueSize++] = entity->id;
		System->Phase[System->PhaseSize++] = entity->id;
		if (IsHostile(entity))
			System->EncounterModeEnabled = true;
	}
}

fn void ClearTurnQueue(turn_system_t *System)
{
	System->QueueSize = 0;
	System->PhaseSize = 0;
}

fn entity_t *Pull(turn_system_t *System)
{
	entity_t *result = 0;
	if (System->QueueSize > 0)
	{
		result = GetEntity(System->storage, System->Queue[System->QueueSize - 1]);
		if (!result)
			System->QueueSize--; // NOTE(): The ID is invalid - pull it from the System.
	}
	return result;
}

fn b32 IsActionQueueCompleted(const turn_system_t *System)
{
	return (System->action_count == 0);
}

fn entity_t *GetActive(const turn_system_t *System)
{
	entity_t *result = 0;
	if (System->QueueSize > 0)
		result = GetEntity(System->storage, System->Queue[System->QueueSize - 1]);
	return result;
}

fn int32_t IsActive(const turn_system_t *System, entity_id_t id)
{
	entity_t *result = GetActive(System);
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
	if (System->turn_inited)
	{
		System->prev_turn_entity = entity->id;
		System->seconds_elapsed = 0.0f;

		if (System->QueueSize > 0)
		{
			System->QueueSize--;
		}

		System->turn_inited = false;
	}
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

	DebugPrint(
		"ACT %i | BRK (F1): %s | GOD (F3): %s | BATTLE: %s | EXIT (F12)",
		(System->action_points),
		(System->break_mode_enabled ? "ON" : "OFF"),
		(System->god_mode_enabled ? "ON" : "OFF"),
		(System->EncounterModeEnabled ? "ON" : "OFF"));
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
			System->Events |= system_any_evicted;
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
	if (System->action_points >= 2 && ((entity->hitchance_boost_multiplier + 0.1f) <= 2.0f))
	{
		entity->has_hitchance_boost = true;
		System->action_points -= 2;
		entity->hitchance_boost_multiplier += 0.1f;
		DebugLog("Used 2 movement points to brace for a hit chance bonus. Hit chance multiplied by %g for next attack.", entity->hitchance_boost_multiplier);
	}
	else if (System->action_points >= 2 && entity->hitchance_boost_multiplier + 0.1f > 2.0f)
	{
		DebugLog("Brace invalid. Your hit chance multiplier for next attack is %g which is maximum.", entity->hitchance_boost_multiplier);
	}
}

/*
fn void UseItem(turn_system_t *State, entity_t *Entity, inventory_t *Eq, item_t Item)
{
	action_type_t Action = Item.params->action;
	if (Action != action_none)
	{
		QueryAsynchronousAction(State, Action, Entity->id, Entity->p);
		Eq_RemoveItem(Eq, Item.ID);
	}
}
*/

fn void ResolveAsynchronousActionQueue(turn_system_t *System, entity_t *user, command_buffer_t *out, f32 dt, assets_t *assets, game_state_t *state)
{
	const map_t *Map = state->Map;

	for (s32 index = 0; index < System->action_count; index++)
	{
		async_action_t *action = &System->actions[index];
		const action_params_t *params = GetParameters(action->action_type.type);

		b32 finished = true;

		// animate
		if (params->animation_ranged)
		{
			// NOTE(): Ranged actions take some amount of time
			// to finish.

			// NOTE(): The duration is proportional to the distance.

			v2 From = GetTileCenter(Map, action->target_p);
			v2 To = user->deferred_p;
			f32 time = action->elapsed / (Distance(From, To) * 0.005f);
			finished = (time >= 1.0f);
			RenderRangedAnimation(out, To, From, params->animation_ranged, time);
		}
		else
		if (params->flags & action_display_move_name)
		{
			f32 time = action->elapsed;
			v2 X = Ease2(40.0f, 60.0f, 15.0f, time);
			f32 alpha = 1.0f - X.y;
			RenderDiegeticText(state->Camera, state->assets->Font, user->deferred_p, V2((-20.0f + X.x), -80.0f), A(White(), alpha), params->name);
			finished =  (time >= 1.0f);
		}

		// commit
		if (finished)
		{
			System->actions[index--] = System->actions[--System->action_count];
			CommitAction(state->System, user, GetEntity(System->storage, action->target_id), &action->action_type, action->target_p);
		}

		action->elapsed += dt * 1.0f;
	}
}

fn void AI(turn_system_t *State, command_buffer_t *Out, entity_t *Entity)
{
	if (!State->EnemyInited)
	{
		range_map_t *Range = &State->EffectiveRange;
		if (Range->FilledCount)
		{
			range_map_cell_t RandomCell = Range->Filled[rand() % Range->FilledCount];
			v2s Target = RandomCell.Cell;

			entity_t *Player = FindClosestPlayer(State->storage, Entity->p);
			if (Player)
			{
				if (CheckRange(&State->EffectiveRange, Player->p))
				{
					Target = Player->p;
				}
			}

			FindPath(State->Map, Entity->p, Target, &State->EnemyPath, *State->Memory);
		}
		State->EnemyInited = true;
	}

	b32 End = true;

	path_t *Path = &State->EnemyPath;
	if (Path->length)
	{
		End = false;

		f32 Speed = 4.0f;
		f32 Edge = 1.0f / (f32)Path->length;
		f32 T = State->EnemyAnimationTime * Edge * Speed;
		T = fclampf(T, 0.0f, 1.0f);
		int32_t Offset = (int32_t)(T * (f32)Path->length);
		Offset = Clamp(Offset, 0, Path->length - 1);

		v2 From = GetTileCenter(State->Map, GetPathTile(Path, Offset));
		v2 To = GetTileCenter(State->Map, GetPathTile(Path, Offset + 1));

		f32 Edge1 = Edge * (float)Offset;
		f32 Edge2 = Edge1 + Edge;

		f32 Step = step(Edge1, Edge2, T);
		v2 Position = Lerp2(From, To, Step);

		Entity->deferred_p = Position;

		if (T >= 1.0f)
		{
			End = true;
		}
	}

	RenderPath(Out, State->Map, Path, W(Red(), 0.5f));
	RenderRangeMap(Out, State->Map, &State->EffectiveRange);

	if (End)
	{
		for (int32_t Index = 0; Index < Path->length; Index++)
		{
			ChangeCell(State, Entity, Path->tiles[Index].p);
		}

		if (IsActionQueueCompleted(State))
		{
			AcceptTurn(State, Entity);
		}
	}
}

fn void BeginTurnSystem(turn_system_t *Queue, game_state_t *Game, f32 dt)
{
	Queue->player 		= Game->player;
	Queue->Map 			= Game->Map;
	Queue->storage 		= Game->storage;
	Queue->particles 	= Game->particles;
	Queue->log 			= Game->log;
	Queue->Memory 		= Game->memory;

	Queue->seconds_elapsed += dt;
	Queue->EnemyAnimationTime += dt;
}

fn void EndTurnSystem(turn_system_t *Queue, game_state_t *Game)
{
	Queue->Events = 0;
}

fn void InteruptTurn(turn_system_t *Queue, entity_t *Entity)
{
	// NOTE(): Alert/Interupt Player Turn
	PushTurn(Queue, Entity);
	Queue->turn_inited = false;
}

fn inline s32 CheckEnemyAlertStates(game_state_t *state, entity_t *ActiveEntity)
{
	turn_system_t *System = state->System;
	s32 Interupted = false;

	entity_storage_t *storage = System->storage;
	for (s32 Index = 0; Index < storage->EntityCount; Index++)
	{
		entity_t *Entity = &storage->entities[Index];
		if (IsHostile(Entity) && (!Entity->Alerted))
		{
			if (IsLineOfSight(state->Map, Entity->p, ActiveEntity->p))
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
		return IsTraversable(System->Map, p);
	}

	return false;
}

fn b32 ChangeCell(turn_system_t *State, entity_t *Entity, v2s NewCell)
{
	b32 Moved = IsCellEmpty(State, NewCell);
	if (Moved)
	{
		StepOnTile(State->Map, Entity);
		Entity->p = NewCell;
	}
	return Moved;
}

fn b32 MakeMove(turn_system_t *System, entity_t *entity, v2s offset)
{
	b32 Moved = ChangeCell(System, entity, IntAdd(entity->p, offset));
	return (Moved);
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
    v2s direction = IntSub(target->p, source);

    if (direction.x != 0) direction.x = (direction.x > 0) ? 1 : -1;
    if (direction.y != 0) direction.y = (direction.y > 0) ? 1 : -1;

    s32 damage_per_tile = strength / (target->size.x * target->size.y);
    s32 total_damage = 0;

    for (s32 i = 0; i < push_distance; ++i) { // todo: make strength affect push_distance somewhat
        v2s next_pos = IntAdd(target->p, direction); // Slime is moved through each tile on the way

        if (!MoveFitsWithSize(System, target, next_pos)) {
            TakeHP(target, (s16)damage_per_tile);
            break;
        } else {
            target->p = next_pos;
            StepOnTile(System->Map, target);
        }
    }

    return false;
}

fn void EstablishTurnOrder(turn_system_t *System)
{
	ClearTurnQueue(System);

	entity_storage_t *Storage = System->storage;
	PushTurn(System, GetEntity(System->storage, System->player));
	
	for (s32 index = 0; index < Storage->EntityCount; index++)
	{
		entity_t *entity = &Storage->entities[index];
		if (IsHostile(entity) && entity->Alerted)
			PushTurn(System, entity);
	}
}

fn void DoDamage(turn_system_t *game, entity_t *user, entity_t *target, s32 damage, const char *damage_type_prefix)
{
    damage = damage + (rand() % 3);
    LogLn(game->log, "%shit! inflicted %i %s of %sdamage upon the target!",
        damage_type_prefix, damage, damage == 1 ? "point" : "points", damage_type_prefix);
    TakeHP(target, (s16)damage);
    CreateDamageNumber(game->particles, Add(target->deferred_p, V2(-25.0f, -25.0f)), damage);

    if ((rand() / (float)RAND_MAX) < 20) {
        blood_type_t blood_type = (target->enemy_type == 0) ? blood_red : blood_green;
        hit_velocity_t hit_velocity = (rand() % 2 == 0) ? high_velocity : low_velocity; // Just a temporary thing until we add ammo types
        BloodSplatter(game->Map, user->p, target->p, blood_type, hit_velocity);
    }
}

fn inline void AOE(turn_system_t *state, entity_t *user, entity_t *target, const action_params_t *params, v2s TileIndex)
{
    entity_storage_t *storage = state->storage;
    
    const char *prefix = "blast ";
    s32 damage = params->damage;
    v2s area = params->area;
    s32 radius_inner = area.x;
    s32 radius_outer = area.x * (s32)2;
    v2s explosion_center = TileIndex;
    for (s32 i = 0; i < storage->EntityCount; i++)
    {
        entity_t *entity = &storage->entities[i];
        f32 distance = IntDistance(explosion_center, entity->p);
        
        if (distance <= radius_inner) {
            DoDamage(state, user, entity, damage, prefix);
        } else if (distance <= radius_outer) {
            DoDamage(state, user, entity, damage, prefix);
            Launch(state, explosion_center, entity, 2, 25);
        }
    }
}

fn inline void SingleTarget(turn_system_t *state, entity_t *user, entity_t *target, const action_params_t *params)
{
    if ((IsPlayer(user) && state->god_mode_enabled))
    {
        DoDamage(state, user, target, target->health, "");
    }
    else
    {
		s32 chance = 100;
		CalculateHitChance(user, target, params->type);

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

fn void CommitAction(turn_system_t *state, entity_t *user, entity_t *target, action_t *action, v2s target_p)
{
    const action_params_t *params = GetParameters(action->type);

    switch (params->mode)
    {
    case action_mode_damage:
    {
        if (IsZero(params->area))
        {
        	if (target)
            	SingleTarget(state, user, target, params);
        }
        else
        {
            AOE(state, user, target, params, target_p);
        }

        // NOTE(): Reset per-turn attack buffs/modifiers.
        user->has_hitchance_boost = false;
        user->hitchance_boost_multiplier = 1.0f;
    } break;
    case action_mode_heal:
    	{
    		Heal(target, (s16) params->value);
    		CreateCombatText(state->particles, target->deferred_p, combat_text_heal);
    	} break;
    case action_mode_dash: user->p = target_p; break;
    }
}

fn void CheckEncounterModeStatus(turn_system_t *System)
{
	s32 AliveHostiles = 0;

	if (System->EncounterModeEnabled)
	{
		DebugLog("checking...");
		
		for (s32 Index = 0; Index < System->PhaseSize; Index++)
		{
			entity_t *Entity = GetEntity(System->storage, System->Phase[Index]);
			if (IsAlive(Entity) && IsHostile(Entity))
				AliveHostiles++;
		}

		if (AliveHostiles == 0)
		{
			System->EncounterModeEnabled = false;
			DebugLog("disengaging encounter mode...");
		}
	}
}