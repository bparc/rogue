int MoveFitsWithSize(game_world_t* world, entity_t *requestee, v2s requestedPos);

fn void EstablishTurnOrder(game_world_t *World, turn_queue_t *queue, entity_storage_t *storage)
{
	DefaultTurnOrder(queue, storage);
}

fn s32 BeginTurn(game_world_t *World, entity_t *requestee)
{
	ProcessStatusEffects(requestee);

	s32 action_point_count = 4;
	if (IsHostile(requestee))
		action_point_count = 4 + (rand() % 2);

	return action_point_count;
}

//make sure enity cant move into wall/does nothing when trapped
fn s32 Decide(game_world_t *World, entity_t *requestee)
{
	int randomIndex = rand() % 4;
	v2s chosenDir = cardinal_directions[randomIndex]; //NOTE(): replaced by enemy ai
	
	int canMove = false;
	int attempts = 0;

	v2s peekPos = AddS(requestee -> p, chosenDir);
	s32 cost = 0;

	while(!canMove && attempts < 5){
		canMove = MoveFitsWithSize(World, requestee, peekPos);
		
		if(canMove) {
			cost = 1; //only costs when can be moved
			MoveEntity(World->map, requestee, chosenDir);
			ApplyTileEffects(requestee->p, World, requestee);
			DebugLog("moving %i %i", peekPos.x, peekPos.y);
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

/*int MoveFitsWithSize(game_world_t* world, v2s size, v2s requestedPos) {
	DebugLog("Peek pos: x=%d, y=%d", requestedPos.x, requestedPos.y);

    for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {
            v2s pos = {requestedPos.x + x, requestedPos.y + y};
        	DebugLog("Checking pos: x=%d, y=%d", pos.x, pos.y);
            if (!IsWorldPointEmpty(world, pos)) {
            	DebugLog("Move doesn't fit at pos: x=%d, y=%d", pos.x, pos.y);
                return false; 
            }
        }
    }
	DebugLog("Move fits at requested position");
    return true;  // Movement is possible
}*/

int MoveFitsWithSize(game_world_t* world, entity_t *requestee, v2s requestedPos) {

    int currentX = requestee->p.x;
    int currentY = requestee->p.y;

    int deltaX = requestedPos.x - currentX; // For example: if requestee is at 8,8 and requestedPos is at 9,9
    int deltaY = requestedPos.y - currentY; //				then delta x will be 9-8=1

    v2s moveCoords[3]; // For diagonal movement there's three coords
    int numCoords = 0;

    // Moving up
    if (deltaX == 0 && deltaY == -1) {
        moveCoords[0] = (v2s){currentX, currentY - 1};     // Tile above (1,1)
        moveCoords[1] = (v2s){currentX + 1, currentY - 1}; // Tile above (2,1)
        numCoords = 2;
    }
    // Moving down
    else if (deltaX == 0 && deltaY == 1) {
        moveCoords[0] = (v2s){currentX, currentY + 2};     // Tile below (1,2)
        moveCoords[1] = (v2s){currentX + 1, currentY + 2}; // Tile below (2,2)
        numCoords = 2;
    }
    // Moving left
    else if (deltaX == -1 && deltaY == 0) {
        moveCoords[0] = (v2s){currentX - 1, currentY};     // Tile to the left (1,1)
        moveCoords[1] = (v2s){currentX - 1, currentY + 1}; // Tile to the left (1,2)
        numCoords = 2;
    }
    // Moving right
    else if (deltaX == 1 && deltaY == 0) {
        moveCoords[0] = (v2s){currentX + 2, currentY};     // Tile to the right (2,1)
        moveCoords[1] = (v2s){currentX + 2, currentY + 1}; // Tile to the right (2,2)
        numCoords = 2;
    }
    // Moving up-left (diagonal)
    else if (deltaX == -1 && deltaY == -1) {
        moveCoords[0] = (v2s){currentX - 1, currentY - 1}; // Top-left
        moveCoords[1] = (v2s){currentX, currentY - 1};     // Top center
        moveCoords[2] = (v2s){currentX - 1, currentY};     // Left center
        numCoords = 3;
    }
    // Moving up-right (diagonal)
    else if (deltaX == 1 && deltaY == -1) {
        moveCoords[0] = (v2s){currentX + 2, currentY - 1}; // Top-right
        moveCoords[1] = (v2s){currentX + 1, currentY - 1}; // Top center
        moveCoords[2] = (v2s){currentX + 2, currentY};     // Right center
        numCoords = 3;
    }
    // Moving down-left (diagonal)
    else if (deltaX == -1 && deltaY == 1) {
        moveCoords[0] = (v2s){currentX - 1, currentY + 2}; // Bottom-left
        moveCoords[1] = (v2s){currentX, currentY + 2};     // Bottom center
        moveCoords[2] = (v2s){currentX - 1, currentY + 1}; // Left center
        numCoords = 3;
    }
    // Moving down-right (diagonal)
    else if (deltaX == 1 && deltaY == 1) {
        moveCoords[0] = (v2s){currentX + 2, currentY + 2}; // Bottom-right
        moveCoords[1] = (v2s){currentX + 1, currentY + 2}; // Bottom center
        moveCoords[2] = (v2s){currentX + 2, currentY + 1}; // Right center
        numCoords = 3;
    } else {
        //DebugLog("Invalid movement of entity id: %d");
        return false;
    }

    for (int i = 0; i < numCoords; i++) {
        if (!IsWorldPointEmpty(world, moveCoords[i])) {
			return false;
        }
    }

    return true;
}