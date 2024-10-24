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
		if (!FindPath(game->map, entity->p, DebugPlayer->p, path, game->memory))
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
	#if 1
	path_t *path = &game->turns->path;
	s32 index = entity->DEBUG_step_count++;
	if (index < path->length)
		entity->p = path->tiles[index].p;
	#else
	int randomIndex = rand() % 4;

	v2s chosenDir;
	if (rand() % 100 < 25) {
		chosenDir = cardinal_directions[randomIndex]; //NOTE(): replaced by enemy ai
	} else {
		chosenDir = GetDirectionToClosestPlayer(World->storage, requestee->p);
	}

	int canMove = false;
	int attempts = 0;

	v2s peekPos = Add32(requestee -> p, chosenDir);

	while(!canMove && attempts < 5){
		canMove = MoveFitsWithSize(World, requestee, peekPos);
		
		if(canMove) {
			Move(World, requestee, chosenDir);
			ApplyTileEffects(requestee->p, World, requestee);
			break;
		}

		peekPos = Add32(requestee -> p, chosenDir);
		chosenDir = cardinal_directions[(randomIndex + 1 ) % 4];
		attempts++;
	}
	#endif
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

fn entity_id_t AttemptAttack(game_world_t *World, entity_t *requestee, s32 effective_range)
{
	entity_id_t result = 0;

	// TODO(): This should be like, a nearest player in range.
	entity_t *target = 0;
	for (s32 index = 0; index < World->storage->num; index++)
	{
		entity_t *entity = &World->storage->entities[index];
		if (entity->flags & entity_flags_controllable)
		{
			target = entity;
			break;
		}
	}

	if (target && IsInsideCircle(target->p, target->size, requestee->p, effective_range))
		result = target->id;
	return result;
}

fn void AnimateAttack(game_world_t *World, entity_t *entity, entity_t *target, f32 time, f32 dt, assets_t *assets, command_buffer_t *out, b32 inflict_damage)
{
	DrawRangedAnimation(out, entity->deferred_p, target->deferred_p, &assets->SlimeBall, time);
	if (inflict_damage)
		TakeHP(target, entity->attack_dmg);
}