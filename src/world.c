//NOTE: static combat items
fn static_entity_t * CreateEffectTile(entity_storage_t *storage, v2s p, v2s size, u8 flags, status_effect_t status_effects[MAX_STATUS_EFFECTS]) {
  static_entity_t *result = 0;
	if (storage->statics_num < ArraySize(storage->static_entities))
		result = &storage->static_entities[storage->statics_num++];
	if (result)
	{
		ZeroStruct(result); //memset macro
		result->p = p;

		result->size = size;
		result->flags = flags;

		//array cpy im killing myself
        for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {
            result->status_effects[i] = status_effects[i];
        }
	}
	
	return result;
}

// NOTE(): Entities.
<<<<<<< Updated upstream
fn entity_t *CreateEntity(entity_storage_t *storage, v2s p, v2s size, u8 flags, u16 health_points,
	u16 attack_dmg, const map_t *map, u16 max_health_points, s32 accuracy, s32 evasion)
{
	entity_t *result = 0;
	if (storage->num < ArraySize(storage->entities))
		result = &storage->entities[storage->num++];
	if (result)
	{
		ZeroStruct(result); //memset macro
		result->p = p;
		result->deferred_p = GetTileCenter(map, result->p);

		result->size = size;

		result->id = (0xff + (storage->next_id++));
		result->flags = flags;
		result->health = health_points;
		result->attack_dmg = attack_dmg;
		result->max_health = max_health_points;
		result->melee_accuracy = accuracy;
		result->ranged_accuracy = accuracy;
		result->evasion = evasion;
	}
	
	return result;
}

=======
>>>>>>> Stashed changes
fn void CreateSlimeI(game_world_t *state, s32 x, s32 y)
{
	u16 slime_hp = 100;
	u16 slime_max_hp = 100;
	u16 slime_attack_dmg = 1;
	s32 slime_accuracy = 30; // Applying this value for both melee and ranged accuracy
	s32 slime_evasion = 80;
	CreateEntity(state->storage, V2S(x, y), V2S(1, 1),  entity_flags_hostile, slime_hp, slime_attack_dmg, state->map, slime_max_hp, slime_accuracy, slime_evasion);
}

fn void CreateBigSlimeI(game_world_t *state, s32 x, s32 y)
{
	u16 slime_hp = 400;
	u16 slime_max_hp = 400;
	u16 slime_attack_dmg = 25;
	s32 slime_accuracy = 45; // Applying this value for both melee and ranged accuracy
	s32 slime_evasion = 40;
	CreateEntity(state->storage, V2S(x, y), V2S(2, 2),  entity_flags_hostile, slime_hp, slime_attack_dmg, state->map, slime_max_hp, slime_accuracy, slime_evasion);
}

fn void CreatePoisonTrapI(game_world_t *state, s32 x, s32 y) {
	u8 flags = static_entity_flags_trap | static_entity_flags_stepon_trigger;

	status_effect_t status_effects = {0};
	status_effects.type = status_effect_poison;
	status_effects.remaining_turns = 3;
	status_effects.damage = 1;

	status_effect_t effects[MAX_STATUS_EFFECTS] = {status_effects, 0, 0};
	CreateEffectTile(state->storage, V2S(x,y), V2S(1,1), flags, effects);
}

fn b32 MoveEntity(map_t *map, entity_t *entity, v2s offset)
{
	entity->p = AddS(entity->p, offset);
	if (entity->p.x < 0)
		entity->p.x = 0;
	if (entity->p.y < 0)
		entity->p.y = 0;
	if (entity->p.x >= map->x)
		entity->p.x = map->x - 1;
	if (entity->p.y >= map->y)
		entity->p.y = map->y - 1;

	return (true);
}

// NOTE(): World.
fn b32 IsWorldPointEmpty(game_world_t *state, v2s p) {

	entity_t *collidingEntity = GetEntityByPosition(state->storage, p);

	if (collidingEntity == NULL) {

		u8 tileValue = GetTileValue(state->map, p.x, p.y);
		b32 result = !(tileValue == 0 || tileValue == 2);
		return !(tileValue == 0 || tileValue == 2); // Wall or out of bounds
	}

	return false; // Entity collision
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

fn void AddStatusEffect(entity_t *entity, status_effect_type_t status_effect, s32 duration) {
    for (int i = 0; i < MAX_STATUS_EFFECTS; ++i) {
        
        if (1 + entity->status_effects[i].type == status_effect_none) { //types start at 1
            
            switch (status_effect) {
                case status_effect_poison:
                    entity->status_effects[i].type = status_effect_poison;
                entity->status_effects[i].remaining_turns = duration;
                break;
                default:
                    break;
            }
        }
    }

}

fn void ApplyTrapEffects(tile_t *tile, entity_t *entity) {
    switch(tile->trap_type) {
        case trap_type_physical:
            InflictDamage(entity, 25);
            break;
        case trap_type_poison:
            AddStatusEffect(entity, status_effect_poison, 3);
            break;
        default:
            break;
    }
}

fn void ApplyTileEffects(v2s peekPos, game_world_t *state, entity_t *entity) {
    tile_t *tile = GetTile(state->map, peekPos.x, peekPos.y);

    if (tile->trap_type != trap_type_none) {
        ApplyTrapEffects(tile, entity);
    }


    // todo: cover mechanics stuff
    //if (tile->cover_type != cover_type_none) {
    //	ApplyCoverBonus(entity);
    //}
}

fn void ProcessStatusEffects(entity_t *entity) {

    for (int i = 0; i < MAX_STATUS_EFFECTS; ++i) {
        if (entity->status_effects[i].type != status_effect_none) {
            switch (entity->status_effects[i].type) {
                case status_effect_poison:
                    InflictDamage(entity, 10);
                    break;
                default:
                    break;
            }

            entity->status_effects[i].remaining_turns--;

            if (entity->status_effects[i].remaining_turns == 0) {
                entity->status_effects[i].type = status_effect_none;
            }
        }
    }
}