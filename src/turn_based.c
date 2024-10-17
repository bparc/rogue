fn void DebugWait(game_world_t *World, entity_t *entity, interpolator_state_t state, command_buffer_t *out)
{

}

fn void EstablishTurnOrder(game_world_t *World, turn_queue_t *queue, entity_storage_t *storage)
{
	DefaultTurnOrder(queue, storage);
}

fn s32 BeginTurn(game_world_t *World, entity_t *entity)
{
	ProcessStatusEffects(entity);

	s32 action_point_count = 4;
	if (IsHostile(entity))
		action_point_count = 2 + (rand() % 2);

	return action_point_count;
}

fn s32 Decide(game_world_t *World, entity_t *requestee)
{
	int randomIndex = rand() % 4;

	v2s chosenDir = cardinal_directions[randomIndex]; //NOTE(): replaced by enemy ai

	#if ENABLE_DEBUG_PATHFINDING
	v2s nearest = FindNearestTile(World->map, requestee->p);
	chosenDir = SubS(nearest, requestee->p);
	MoveEntity(World->map, requestee, chosenDir);
	#else

	int canMove = false;
	int attempts = 0;

	v2s peekPos = AddS(requestee -> p, chosenDir);

	while(!canMove && attempts < 5){
		canMove = MoveFitsWithSize(World, requestee, peekPos);
		
		if(canMove) {
			MoveEntity(World->map, requestee, chosenDir);
			ApplyTileEffects(requestee->p, World, requestee);
			//DebugLog("moving %i %i", peekPos.x, peekPos.y);
			break;
		}

		peekPos = AddS(requestee -> p, chosenDir);
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

fn void AnimateAttack(game_world_t *World, entity_t *entity, entity_t *target, f32 time, f32 dt, assets_t *assets, command_buffer_t *out, b32 inflict_damage){
	v2 from = entity->deferred_p;
	v2 to = Sub(target->deferred_p, V2(20.0f, 20.0f));

	// TODO(): Move this out to RenderCenteredIsoBitmap() or whatnot.
	
	if (inflict_damage)
		InflictDamage(target, entity->attack_dmg);

	bitmap_t *bitmap = &assets->SlimeBall;
	v2 bitmap_p = Lerp2(from, to, time);
	bitmap_p = ScreenToIso(bitmap_p);
	bitmap_p = Sub(bitmap_p, Scale(bitmap->scale, 0.5f));
	DrawBitmap(out, bitmap_p, bitmap->scale,  PureWhite(), bitmap);
}