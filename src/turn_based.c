int MoveFitsWithSize(game_world_t* world, v2s size, v2s requestedPos);

fn void EstablishTurnOrder(game_world_t *World, turn_queue_t *queue, entity_storage_t *storage)
{
	DefaultTurnOrder(queue, storage);
}

fn s32 BeginTurn(game_world_t *World, entity_t *requestee)
{
	ProcessStatusEffects(requestee);

	s32 action_point_count = 8;
	if (IsHostile(requestee))
		action_point_count = 4 + (rand() % 2);

	return action_point_count;
}

//make sure enity cant move into wall/does nothing when trapped
fn s32 Decide(game_world_t *World, entity_t *requestee)
{
	int randomIndex = rand() % 4;
	v2s chosenDir = cardinal_directions[randomIndex]; //NOTE(): replaced by enemy ai
	
	int cantMove = false;
	int attempts = 0;

	v2s peekPos = AddS(requestee -> p, chosenDir);;
	s32 cost = 0;

	while(!cantMove && attempts < 5){
		
		cantMove = (!IsOutOfBounds(World, peekPos) && !IsWall(World, peekPos) && MoveFitsWithSize(World, requestee->size, peekPos));
		
		if(cantMove) {
			cost = 1; //only costs when can be moved
			MoveEntity(World->map, requestee, chosenDir);
			ApplyTileEffects(requestee->p, World, requestee);
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
	if (target)
	{
		// InflictDamage(requestee, target);
		result = true;
	}
	return result;
}

int MoveFitsWithSize(game_world_t* world, v2s size, v2s requestedPos) {
    for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {
            v2s pos = {requestedPos.x + x, requestedPos.y + y};
            if (IsOutOfBounds(world, pos) || IsWall(world, pos)) {
                return false; 
            }
        }
    }
    return true;  // Movement is possible
}