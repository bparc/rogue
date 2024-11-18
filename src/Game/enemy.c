fn void EstablishTurnOrder(game_state_t *game, turn_system_t *queue)
{
	entity_storage_t *Storage = queue->storage;
	ClearTurnQueue(queue);

	for (s32 index = 0; index < Storage->EntityCount; index++)
	{
		entity_t *entity = &Storage->entities[index];
		if (IsHostile(entity) == false)
			PushTurn(queue, entity);
	}

	for (s32 index = 0; index < Storage->EntityCount; index++)
	{
		entity_t *entity = &Storage->entities[index];
		if (IsHostile(entity) && entity->Alerted)
			PushTurn(queue, entity);
	}
}

fn s32 BeginTurn(game_state_t *game, entity_t *entity)
{
	ProcessStatusEffects(entity);

	s32 movement_point_count = 10;
	if (IsHostile(entity))
	{
		//movement_point_count = 2 + (rand() % 2);

		// NOTE(): Request a path to the player.
		turn_system_t *queue = game->turns;
		entity_t *player = FindClosestPlayer(game->storage, entity->p);
		Assert(player);

		path_t *path = &queue->path;
		if (!FindPath(game->map, entity->p, player->p, path, *game->memory))
			DebugLog("Couldn't find a path!");
		movement_point_count = 6;
		queue->max_action_points = movement_point_count;

		path->length = Min32(path->length, movement_point_count);

		// NOTE(): Truncate path to the closest unoccupied point to the
		// destination.
		s32 index = path->length - 1;
		while (index >= 0)
		{
			if (IsWorldPointEmpty(game->turns, path->tiles[index].p) == false)
			{
				index--;
				continue;
			}
			break;
		}
		movement_point_count = path->length = (index + 1);
		
		entity->DEBUG_step_count = 0;
	}

	return movement_point_count;
}

fn s32 Decide(game_state_t *game, entity_t *entity)
{
	path_t *path = &game->turns->path;
	s32 index = entity->DEBUG_step_count++;
	if (index < path->length)
		entity->p = path->tiles[index].p;
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