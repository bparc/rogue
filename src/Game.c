fn entity_t *CreateEntity(game_state_t *State, v2s p, v2s size, u8 flags, u16 health_points, u16 attack_dmg, const map_t *Map, u16 max_health_points, s32 accuracy, s32 evasion, s32 remaining_action_points, s32 remaining_movement_points, f32 hitchance_boost_multiplier)
{
	entity_storage_t *storage = &State->Units;
	entity_t *result = 0;
	if (storage->EntityCount < ArraySize(storage->entities))
		result = &storage->entities[storage->EntityCount++];
	if (result)
	{
		ZeroStruct(result); //memset macro
		result->p = p;
		result->deferred_p = GetTileCenter(Map, result->p);

		result->size = size;

		result->id = (0xff + (storage->IDPool++));
		result->flags = flags;
		result->health = health_points;
		result->attack_dmg = attack_dmg;
		result->max_health = max_health_points;
		result->melee_accuracy = accuracy;
		result->ranged_accuracy = accuracy;
		result->evasion = evasion;
		result->hitchance_boost_multiplier = hitchance_boost_multiplier;
	}
	
	return result;
}

fn container_t *CreateContainer(game_state_t *State, v2s Pos)
{
    container_t *result = 0;
    if ((State->Units.ContainerCount < ArraySize(State->Units.Containers)) &&
    	InMapBounds(&State->Map, Pos))
    {
        result = &State->Units.Containers[State->Units.ContainerCount++];
        ZeroStruct(result);
        result->ID = State->Units.ContainerCount;

        State->Map.container_ids[GetTileIndex(&State->Map, Pos)] = result->ID;
        SetupInventory(&result->inventory, &State->GlobalItemCount);
    }
    return result;
}

fn inventory_t *CreateInventory(game_state_t *State)
{
	inventory_t *Result = PushStruct(inventory_t, State->Memory);
	SetupInventory(Result, &State->GlobalItemCount);
	return Result;
}

fn container_t *GetContainer(game_state_t *State, v2s position)
{
    entity_storage_t *Storage = &State->Units;
    map_t *Map = &State->Map;

    container_t *Result = 0;
    if (InMapBounds(&State->Map, position))
    {
        s32 TileIndex = GetTileIndex(&State->Map, position);
        s32 ContainerIndex = (Map->container_ids[TileIndex] - 1);
        if ((ContainerIndex >= 0) && (ContainerIndex < Storage->ContainerCount))
            Result = &Storage->Containers[ContainerIndex];
    }
    return Result;
}

fn void PushTurn(game_state_t *State, entity_t *entity)
{
	if ((State->QueueSize < ArraySize(State->Queue)) && entity)
	{
		State->Queue[State->QueueSize++] = entity->id;
		State->Phase[State->PhaseSize++] = entity->id;
		if (IsHostile(entity))
			State->EncounterModeEnabled = true;
	}
}

fn void ClearTurnQueue(game_state_t *State)
{
	State->QueueSize = 0;
	State->PhaseSize = 0;
}

fn inline entity_t *__pull(game_state_t *State)
{
	entity_t *result = 0;
	if (State->QueueSize > 0)
	{
		result = GetEntity(&State->Units, State->Queue[State->QueueSize - 1]);
		if (!result)
			State->QueueSize--; // NOTE(): The ID is invalid - pull it from the State.
	}
	return result;
}

fn b32 IsActionQueueCompleted(const game_state_t *State)
{
	return (State->ActionCount == 0);
}

fn entity_t *GetActive(const game_state_t *State)
{
	entity_t *result = 0;
	if (State->QueueSize > 0)
		result = GetEntity((entity_storage_t *)&State->Units, State->Queue[State->QueueSize - 1]);
	return result;
}

fn int32_t IsActive(const game_state_t *State, entity_id_t id)
{
	entity_t *result = GetActive(State);
	if (result)
		return (result->id == id);
	return 0;
}

fn entity_t *PeekNextTurn(game_state_t *State, entity_storage_t *storage)
{
	entity_t *result = 0;
	if (State->QueueSize >= 2)
		result = GetEntity(storage, State->Queue[State->QueueSize - 2]);
	return result;
}

fn void EndTurn(game_state_t *State, entity_t *entity)
{
	if (State->TurnInited)
	{
		State->PrevTurnEntity = entity->id;
		State->SecondsElapsed = 0.0f;

		if (State->QueueSize > 0)
		{
			State->QueueSize--;
		}

		State->TurnInited = false;
	}
}

fn s32 ConsumeActionPoints(game_state_t *State, s32 count)
{
	s32 sufficient = State->ActionPoints - count >= 0;
	if (sufficient)
		State->ActionPoints -= count;
	else
		DebugLog("Insufficient amount of action points! (%i req.)", count);
	return sufficient;
}

fn void QueryAsynchronousAction(game_state_t *State, action_type_t type, entity_id_t target, v2s target_p)
{
	async_action_t *result = 0;
	if ((State->ActionCount < ArraySize(State->Actions)))
	{
		result = &State->Actions[State->ActionCount++];
		
		ZeroStruct(result);
		result->target_id = target;
		result->target_p  = target_p;
		
		result->action_type.type = type;
	}
}

fn void AnimateAction(const game_state_t *State, const entity_t *Entity, async_action_t *Act, command_buffer_t *Out, f32 dt)
{
	f32 StartTime = 0.00f;
	f32 Elapsed = 2.0f;
	
	const action_params_t *Type = GetActionParams(Act->action_type.type);
	
	if (Type->animation_ranged)
	{
		f32 Speed = 5.0f;

		v2 A = GetTileCenter(&State->Map, Entity->p);
		v2 B = GetTileCenter(&State->Map, Act->target_p);
		Elapsed = Speed * (1.0f / Distance(A, B));

		RenderRangedAnimation(Out, A, B, Type->animation_ranged, Act->Lerp);
	}
	else
	{
		Elapsed = 2.0f; // instantaneous
	}

	Act->Lerp += Elapsed;
}

fn void ControlPanel(game_state_t *State, const virtual_controls_t *cons)
{
	DebugPrint("ACT %i | BRK (??): OFF | BATTLE: %s | EXIT (F12)",
		(State->ActionPoints),
		(State->EncounterModeEnabled ? "ON" : "OFF"));
}

fn evicted_entity_t *GetEvictedEntity(game_state_t *State, entity_id_t ID)
{
	for (s32 index = 0; index < State->EvictedEntityCount; index++)
	{
		evicted_entity_t *entity = &State->EvictedEntities[index];
		if (entity->id == ID)
			return entity;
	}
	return NULL;
}

typedef struct
{
	int32_t DeletedEntityCount;
} garbage_collect_result_t;

fn garbage_collect_result_t GarbageCollect(game_state_t *Game, game_state_t *State, f32 dt)
{
	garbage_collect_result_t Result = {0};

	entity_storage_t *storage = &State->Units;
	for (s32 index = 0; index < storage->EntityCount; index++)
	{
		entity_t *entity = &storage->entities[index];
		if (entity->flags & entity_flags_deleted)
		{
			Perish(State, entity);

			if (State->EvictedEntityCount < ArraySize(State->EvictedEntities))
			{
				evicted_entity_t *evicted = &State->EvictedEntities[State->EvictedEntityCount++];
				evicted->entity = *entity;
				evicted->time_remaining = 1.0f;
			}

			storage->entities[index--] = storage->entities[--storage->EntityCount];

			Result.DeletedEntityCount++;
		}
	}

	for (s32 index = 0; index < State->EvictedEntityCount; index++)
	{
		evicted_entity_t *entity = &State->EvictedEntities[index];
		entity->time_remaining -= (dt * ENTITY_EVICTION_SPEED);
		if (entity->time_remaining <= 0.0f)
			State->EvictedEntities[index--] = State->EvictedEntities[--State->EvictedEntityCount];
	}

	return Result;
}

fn void Brace(game_state_t *State, entity_t *entity)
{
	if (State->ActionPoints >= 2 && ((entity->hitchance_boost_multiplier + 0.1f) <= 2.0f))
	{
		entity->has_hitchance_boost = true;
		State->ActionPoints -= 2;
		entity->hitchance_boost_multiplier += 0.1f;
		DebugLog("Used 2 movement points to brace for a hit chance bonus. Hit chance multiplied by %g for next attack.", entity->hitchance_boost_multiplier);
	}
	else if (State->ActionPoints >= 2 && entity->hitchance_boost_multiplier + 0.1f > 2.0f)
	{
		DebugLog("Brace invalid. Your hit chance multiplier for next attack is %g which is maximum.", entity->hitchance_boost_multiplier);
	}
}

fn void RemoveMovementRange(game_state_t *State)
{
	ClearRangeMap(&State->EffectiveRange);
}

fn void UpdateAI(game_state_t *State, entity_t *Entity)
{
	if (!State->EnemyInited)
	{
		range_map_t *Range = &State->EffectiveRange;
		if (Range->FilledCount)
		{
			range_map_cell_t RandomCell = Range->Filled[rand() % Range->FilledCount];
			v2s Target = RandomCell.Cell;

			entity_t *UpdatePlayer = FindClosestPlayer(&State->Units, Entity->p);
			if (UpdatePlayer)
			{
				if (CheckRange(&State->EffectiveRange, UpdatePlayer->p))
				{
					Target = UpdatePlayer->p;
				}
			}

			FindPath(&State->Map, Entity->p, Target, &State->EnemyPath, *State->Memory);
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
		f32 T = State->EnemyLerp * Edge * Speed;
		T = fclampf(T, 0.0f, 1.0f);
		int32_t Offset = (int32_t)(T * (f32)Path->length);
		Offset = Clamp(Offset, 0, Path->length - 1);

		v2 From = GetTileCenter(&State->Map, GetPathTile(Path, Offset));
		v2 To = GetTileCenter(&State->Map, GetPathTile(Path, Offset + 1));

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

	if (End)
	{
		for (int32_t Index = 0; Index < Path->length; Index++)
		{
			ChangeCell(State, Entity, Path->tiles[Index].p);
		}

		if (IsActionQueueCompleted(State))
		{
			EndTurn(State, Entity);
		}
	}
}

fn inline void _PopRemovedUnits(game_state_t *State)
{
	while (State->QueueSize > 0)
	{
		entity_t *Top = GetEntity(&State->Units, State->Queue[State->QueueSize - 1]);
		if (!Top)
		{
			State->QueueSize--;
			continue;
		}

		break;
	}
}

fn void BeginTurnSystem(game_state_t *State, game_state_t *Game, f32 dt)
{
	State->SecondsElapsed += dt;
	State->EnemyLerp += dt;

	_PopRemovedUnits(State);
}

fn void EndTurnSystem(game_state_t *State, game_state_t *Game)
{
	
}

fn void InteruptTurn(game_state_t *State, entity_t *Entity)
{
	State->TurnInited = false;
	PushTurn(State, Entity);
}

fn inline s32 CheckEnemyAlertStates(game_state_t *State, entity_t *ActiveEntity)
{
	s32 Interupted = false;

	entity_storage_t *storage = &State->Units;
	for (s32 Index = 0; Index < storage->EntityCount; Index++)
	{
		entity_t *Entity = &storage->entities[Index];
		if (IsHostile(Entity) && (!Entity->Alerted))
		{
			if (IsLineOfSight(&State->Map, Entity->p, ActiveEntity->p))
			{
				CreateCombatText(&State->ParticleSystem, Entity->deferred_p, combat_text_alerted);
				Entity->Alerted = true;

				InteruptTurn(State, Entity);
				Interupted = true;
				break;
			}
		}
	}

	return (Interupted);
}

fn b32 IsCellEmpty(game_state_t *State, v2s p)
{
	entity_t *collidingEntity = GetEntityFromPosition(&State->Units, p);

	if (collidingEntity == NULL)
    {
		return IsTraversable(&State->Map, p);
	}

	return false;
}

fn void StepOnTile(game_state_t *State, entity_t *entity)
{
	tile_t *tile = GetTile(&State->Map, entity->p.x, entity->p.y);
	if (tile && (tile->trap_type != trap_type_none))
	{
    	switch(tile->trap_type)
    	{
    	    case trap_type_physical:
    	        TakeHP(entity, 25);
    	        break;
    	    case trap_type_poison:
    	        AddStatusEffect(State, entity, status_effect_poison, 3);
    	        break;
    	    default:
    	        break;
    	}
	}
}

fn b32 ChangeCell(game_state_t *State, entity_t *Entity, v2s NewCell)
{
	b32 Moved = IsCellEmpty(State, NewCell);
	if (Moved)
	{
		StepOnTile(State, Entity);
		Entity->p = NewCell;
	}
	return Moved;
}

fn b32 MakeMove(game_state_t *State, entity_t *entity, v2s offset)
{
	b32 Moved = ChangeCell(State, entity, IntAdd(entity->p, offset));
	return (Moved);
}

fn int MoveFitsWithSize(game_state_t* State, entity_t *requestee, v2s requestedPos)
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
        if (!IsCellEmpty(State, moveCoords[i])) {
			return false;
        }
    }

    return true;
}

fn b32 Launch(game_state_t *State, v2s source, entity_t *target, u8 push_distance, s32 strength)
{
    v2s direction = IntSub(target->p, source);

    if (direction.x != 0) direction.x = (direction.x > 0) ? 1 : -1;
    if (direction.y != 0) direction.y = (direction.y > 0) ? 1 : -1;

    s32 damage_per_tile = strength / (target->size.x * target->size.y);
    s32 total_damage = 0;

    for (s32 i = 0; i < push_distance; ++i) { // todo: make strength affect push_distance somewhat
        v2s next_pos = IntAdd(target->p, direction); // Slime is moved through each tile on the way

        if (!MoveFitsWithSize(State, target, next_pos)) {
            TakeHP(target, (s16)damage_per_tile);
            break;
        } else {
            target->p = next_pos;
            StepOnTile(State, target);
        }
    }

    return false;
}

fn void EstablishTurnOrder(game_state_t *State)
{
	ClearTurnQueue(State);

	entity_storage_t *Storage = &State->Units;
	PushTurn(State, GetEntity(&State->Units, State->Players[0]));
	
	for (s32 index = 0; index < Storage->EntityCount; index++)
	{
		entity_t *entity = &Storage->entities[index];
		if (IsHostile(entity) && entity->Alerted)
			PushTurn(State, entity);
	}
}

fn void CheckEncounterModeStatus(game_state_t *State)
{
	if (State->EncounterModeEnabled)
	{
		DebugLog("...");
		
		s32 HostileCount = 0;
		for (s32 Index = 0; Index < State->PhaseSize; Index++)
		{
			entity_t *Entity = GetEntity(&State->Units, State->Phase[Index]);
			if (IsAlive(Entity) && IsHostile(Entity))
				HostileCount++;
		}

		if (HostileCount == 0)
		{
			State->EncounterModeEnabled = false;
			DebugLog("EncounterModeEnabled = false;");
		}
	}
}

fn b32 SetupTurn(game_state_t *State, s32 ActionPointCount)
{
	State->ActionPoints = ActionPointCount;

	State->SecondsElapsed = 0.0f;

	State->EnemyLerp = 0.0f;
	State->EnemyInited = false;

	return (true);
}