fn void EstablishTurnOrder(game_world_t *game, turn_queue_t *queue)
{
	DefaultTurnOrder(queue);
}

fn s32 BeginTurn(game_world_t *game, entity_t *entity)
{
	ProcessStatusEffects(entity);

	s32 movement_point_count = 10;
	if (IsHostile(entity))
	{
		//movement_point_count = 2 + (rand() % 2);

		// NOTE(): Request a path to the player.
		turn_queue_t *queue = game->turns;
		entity_t *DebugPlayer = DEBUGGetPlayer(queue->storage);
		Assert(DebugPlayer);

		path_t *path = &queue->path;
		if (!FindPath(game->map, entity->p, DebugPlayer->p, path, *game->memory))
			DebugLog("Couldn't find a path!");
		movement_point_count = 6;
		queue->max_action_points = movement_point_count;

		path->length = Min32(path->length, movement_point_count);

		// NOTE(): Truncate path to the closest unoccupied point to the
		// destination.
		s32 index = path->length - 1;
		while (index >= 0)
		{
			if (IsWorldPointEmpty(game, path->tiles[index].p) == false)
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

fn s32 Decide(game_world_t *game, entity_t *entity)
{
	path_t *path = &game->turns->path;
	s32 index = entity->DEBUG_step_count++;
	if (index < path->length)
		entity->p = path->tiles[index].p;
	return 1;
}

fn void Perish(game_world_t *game, entity_t *entity)
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

fn b32 ScheduleEnemyAction(game_world_t *World, entity_t *requestee, s32 effective_range)
{
	turn_queue_t *queue = World->turns;
	entity_id_t result = 0;

	// TODO(): This should be like, a nearest player in range.
	entity_t *target = DEBUGGetPlayer(World->storage);
	if (target && IsInsideCircle(target->p, target->size, requestee->p, effective_range))
	{
		action_type_t action_type = action_none;
		if (RandomChance(30))
			action_type = action_slash;
		else
			action_type = action_slime_ranged;

		// TODO(Arc): LINE OF SIGHT TEST (etc.) GOES HERE.
		b32 LOSTest = true;
		if (LOSTest)
		{
			QueryAsynchronousAction(queue, action_type, target->id, target->p);
			result = target->id;
		}
	}
	return true;
}