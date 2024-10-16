fn void DebugWait(game_world_t *World, entity_t *entity, interpolator_state_t state, command_buffer_t *out)
{

}

fn void EstablishTurnOrder(game_world_t *World, turn_queue_t *queue, entity_storage_t *storage)
{
	DefaultTurnOrder(queue, storage);
}

fn s32 BeginTurn(game_world_t *World, entity_t *requestee)
{
	ProcessStatusEffects(requestee);

	s32 action_point_count = 3;
	if (IsHostile(requestee))
		action_point_count = 2 + (rand() % 2);

	return action_point_count;
}

fn s32 Decide(game_world_t *World, entity_t *requestee)
{
	int randomIndex = rand() % 4;
	v2s chosenDir = cardinal_directions[randomIndex]; //NOTE(): replaced by enemy ai
	
	int canMove = false;
	int attempts = 0;

	v2s peekPos = AddS(requestee -> p, chosenDir);
	s32 cost = 1;

	while(!canMove && attempts < 5){
		canMove = MoveFitsWithSize(World, requestee, peekPos);
		
		if(canMove) {
			cost = 1; //only costs when can be moved
			MoveEntity(World->map, requestee, chosenDir);
			ApplyTileEffects(requestee->p, World, requestee);
			//DebugLog("moving %i %i", peekPos.x, peekPos.y);
			break;
		}

		peekPos = AddS(requestee -> p, chosenDir);
		chosenDir = cardinal_directions[(randomIndex + 1 ) % 4];
		attempts++;
	}

	return (cost);
}

fn s32 AttemptAttack(game_world_t *World, entity_t *requestee)
{
	// NOTE(): Return true if the attack was successuful.
	// We could be also returning here some 
	// additional information regarding the attack move that
	// took place, but we'll see.
	s32 result = true;

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

	if (target)
	{
		DebugLog("target found #%i", target->id);
		InflictDamage(target, requestee->attack_dmg);
		result = true;
	}
	return result;
}