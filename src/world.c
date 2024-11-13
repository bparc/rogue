#define MAX_SLIME_ACTION_POINTS 10
#define MAX_SLIME_MOVEMENT_POINTS 10
#define MAX_PLAYER_ACTION_POINTS 4
#define MAX_PLAYER_MOVEMENT_POINTS 4

fn entity_t *CreatePlayer(game_world_t *state, v2s p)
{
    u16 player_health = 62;
    u16 player_max_health = 62;
    u16 attack_dmg = 8;
    s32 player_accuracy = 75; // Applying this value for both melee and ranged accuracy
    s32 player_evasion = 20;
    entity_t *result = CreateEntity(state->storage, p, V2S(1, 1), entity_flags_controllable,
        player_health, attack_dmg, state->map, player_max_health, player_accuracy, player_evasion,
        MAX_PLAYER_ACTION_POINTS, MAX_PLAYER_MOVEMENT_POINTS, 1);
    
	result->inventory = PushStruct(inventory_t, state->memory);
    SetupInventory(result->inventory);

    inventory_t *Eq = result->inventory;
    Eq_AddItem(Eq, item_green_herb);
    Eq_AddItem(Eq, item_green_herb);
    Eq_AddItem(Eq, item_assault_rifle);
    Eq_AddItem(Eq, item_green_herb);
    Eq_AddItem(Eq, item_assault_rifle);
    Eq_AddItem(Eq, item_green_herb);
    Eq_AddItem(Eq, item_green_herb);
    Eq_AddItem(Eq, item_green_herb);
    Eq_AddItem(Eq, item_green_herb);
    return result;
}

fn entity_t *CreateSlime(game_world_t *state, v2s p)
{
	u16 slime_hp = 54;
	u16 slime_max_hp = 54;
	u16 slime_attack_dmg = 6;
	s32 slime_accuracy = 30; // Applying this value for both melee and ranged accuracy
	s32 slime_evasion = 80;
	entity_t *result = CreateEntity(state->storage, p, V2S(1, 1),  entity_flags_hostile, slime_hp, slime_attack_dmg, state->map,
	slime_max_hp, slime_accuracy, slime_evasion, MAX_SLIME_ACTION_POINTS, MAX_SLIME_MOVEMENT_POINTS, 1);
    result->enemy_type = enemy_slime;
    return result;
}

fn void CreateBigSlime(game_world_t *state, v2s p)
{
	u16 slime_hp = 400;
	u16 slime_max_hp = 400;
	u16 slime_attack_dmg = 25;
	s32 slime_accuracy = 45; // Applying this value for both melee and ranged accuracy
	s32 slime_evasion = 40;
	entity_t *result = CreateEntity(state->storage, p, V2S(2, 2),  entity_flags_hostile, slime_hp, slime_attack_dmg, state->map,
	slime_max_hp, slime_accuracy, slime_evasion, MAX_SLIME_ACTION_POINTS, MAX_SLIME_MOVEMENT_POINTS, 1);
    result->enemy_type = enemy_slime_large;
}

fn void CreatePoisonTrap(game_world_t *state, v2s p) {
	u8 flags = static_entity_flags_trap | static_entity_flags_stepon_trigger;

	status_effect_t status_effects = {0};
	status_effects.type = status_effect_poison;
	status_effects.remaining_turns = 3;
	status_effects.damage = 1;

	status_effect_t effects[MAX_STATUS_EFFECTS] = {status_effects, 0, 0};
	CreateStaticEntity(state->storage, p, V2S(1,1), flags, effects);
}

fn b32 IsWorldPointEmpty(game_world_t *state, v2s p)
{
	entity_t *collidingEntity = GetEntityByPosition(state->storage, p);

	if (collidingEntity == NULL) {

		u8 tileValue = GetTileValue(state->map, p.x, p.y);
		b32 result = !(tileValue == 0 || tileValue == 2);
		return !(tileValue == 0 || tileValue == 2); // Wall or out of bounds
	}

	return false; // Entity collision
}

fn b32 Move(game_world_t *world, entity_t *entity, v2s offset)
{
	v2s requested_p = Add32(entity->p, offset);
	b32 valid = IsWorldPointEmpty(world, requested_p);
	if (valid)
		entity->p = requested_p;
	return (valid);
}

int MoveFitsWithSize(game_world_t* world, entity_t *requestee, v2s requestedPos)
{
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