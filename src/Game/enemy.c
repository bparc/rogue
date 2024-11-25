fn void FindPathToEntity(turn_system_t *State, v2s From, entity_t *Entity, path_t *Path, s32 MaxLength, memory_t Memory)
{
	if (Entity)
	{
		FindPath(State->map, From, Entity->p, Path, Memory);
		Path->length = Min32(Path->length, MaxLength);

		// truncate to the closest unoccupied point to the entity
		s32 index = Path->length - 1;
		while (index >= 0)
		{
			if (IsCellEmpty(State, Path->tiles[index].p) == false)
			{
				index--;
				continue;
			}
			break;
		}
	
		Path->length = (index + 1);
	}
}

fn s32 BeginEnemyTurn(turn_system_t *State, entity_t *entity, memory_t Memory)
{
	s32 movement_point_count = 6;
	FindPathToEntity(State, entity->p, FindClosestPlayer(State->storage, entity->p), &State->path, 16, Memory);

	entity->DEBUG_step_count = 0;
	return (movement_point_count);
}

fn s32 Decide(turn_system_t *System, entity_t *entity)
{
	path_t *path = &System->path;
	s32 CurrentNode = entity->DEBUG_step_count++;
	if (CurrentNode < path->length)
	{
		entity->p = path->tiles[CurrentNode].p;
	}
	else
	{
		System->action_points = 0; // Stop
	}
	return 1;
}

fn inline void SubdivideLargeSlime(game_state_t *game, entity_t *entity, s32 x, s32 y)
{
    entity_t *result = CreateSlime(game, Add32(entity->p, V2S(x, y)));
    if (result)
        result->deferred_p = entity->deferred_p;
}

fn void Perish(game_state_t *game, entity_t *entity)
{
	switch (entity->enemy_type)
	{
	case enemy_slime_large:
		{
			SubdivideLargeSlime(game, entity, 0, 0);
			SubdivideLargeSlime(game, entity, 1, 0);
			SubdivideLargeSlime(game, entity, 1, 1);
			SubdivideLargeSlime(game, entity, 0, 1);
		} break;
	}
}

fn b32 ScheduleEnemyAction(game_state_t *World, entity_t *requestee, s32 effective_range)
{
	turn_system_t *queue = World->turns;
	entity_id_t result = 0;

	entity_t *target = FindClosestPlayer(World->storage, requestee->p);
	if (target && IsInsideCircle(target->p, target->size, requestee->p, effective_range))
	{
		action_type_t action_type = action_none;
		if (RandomChance(30))
			action_type = action_slash;
		else
			action_type = action_slime_ranged;
		
		if (IsLineOfSight(World->map, requestee->p, target->p))
		{
			QueryAsynchronousAction(queue, action_type, target->id, target->p);
			result = target->id;
		}
	}
	return true;
}