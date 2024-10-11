fn void EstablishTurnOrder(game_world_t *World, turn_queue_t *queue, entity_storage_t *storage)
{
	DefaultTurnOrder(queue, storage);
}

fn s32 BeginTurn(game_world_t *World, entity_t *requestee)
{
	s32 action_point_count = 8;
	if (IsHostile(requestee))
		action_point_count = 1 + (rand() % 2);

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
		
		cantMove = (!IsOutOfBounds(World, peekPos) && !IsWall(World, peekPos));
		
		if(cantMove) {
			cost = 1; //only costs when can be moved
			MoveEntity(World->map, requestee, chosenDir);
			break;
		}

		peekPos = AddS(requestee -> p, chosenDir);
		chosenDir = cardinal_directions[(randomIndex + 1 ) % 4];
		attempts++;
		
	}

	
	
	return (cost);
}