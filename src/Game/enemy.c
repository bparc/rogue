fn void FindPathToEntity(game_state_t *State, v2s From, entity_t *Entity, path_t *Path, s32 MaxLength, memory_t Memory)
{
	if (Entity)
	{
		FindPath(&State->Map, From, Entity->p, Path, Memory);
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

fn s32 BeginEnemyTurn(game_state_t *State, entity_t *entity, memory_t Memory)
{
	s32 movement_point_count = 6;
	return (movement_point_count);
}

fn inline void SubdivideLargeSlime(game_state_t *game, entity_t *entity, s32 x, s32 y)
{
    entity_t *result = CreateSlime(game, IntAdd(entity->p, V2S(x, y)));
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