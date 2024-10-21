fn void DebugWait(game_world_t *World, entity_t *entity, interpolator_state_t state, command_buffer_t *out)
{

}

fn void EstablishTurnOrder(game_world_t *World, turn_queue_t *queue)
{
	DefaultTurnOrder(queue);
}

fn s32 BeginTurn(game_world_t *World, entity_t *entity)
{
	ProcessStatusEffects(entity);

	s32 action_point_count = 10;
	if (IsHostile(entity))
		action_point_count = 2 + (rand() % 2);

	return action_point_count;
}

fn void SubdivideLargeSlime(game_world_t *game, entity_t *entity, s32 x, s32 y)
{
	entity_t *result = CreateSlime(game, Add32(entity->p, V2S(x, y)));
	if (result)
		result->deferred_p = entity->deferred_p;
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

fn s32 Decide(game_world_t *World, entity_t *requestee)
{
	int randomIndex = rand() % 4;

	v2s chosenDir;
	if (rand() % 100 < 25) {
		chosenDir = cardinal_directions[randomIndex]; //NOTE(): replaced by enemy ai
	} else {
		chosenDir = GetDirectionToClosestPlayer(World->storage, requestee->p);
	}

	#if ENABLE_DEBUG_PATHFINDING
	v2s nearest = FindNearestTile(World->map, requestee->p);
	chosenDir = Sub32(nearest, requestee->p);
	Move(World->map, requestee, chosenDir);
	#else

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
	{
		result = target->id;
		DebugLog("target found #%i", target->id);
	}
	return result;
}

fn void AnimateAttack(game_world_t *World, entity_t *entity, entity_t *target, f32 time, f32 dt, assets_t *assets, command_buffer_t *out, b32 inflict_damage)
{
	DrawRangedAnimation(out, entity->deferred_p, target->deferred_p, &assets->SlimeBall, time);
	if (inflict_damage)
		InflictDamage(target, entity->attack_dmg);
}