fn b32 IsWorldPointEmpty(game_state_t *state, v2s p)
{
	entity_t *collidingEntity = GetEntityByPosition(state->storage, p);

	if (collidingEntity == NULL)
    {
		return IsTraversable(state->map, p);
	}

	return false;
}

fn b32 Move(game_state_t *world, entity_t *entity, v2s offset)
{
	v2s requested_p = Add32(entity->p, offset);
	b32 valid = IsWorldPointEmpty(world, requested_p);
	if (valid)
		entity->p = requested_p;
	return (valid);
}

int MoveFitsWithSize(game_state_t* world, entity_t *requestee, v2s requestedPos)
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

fn void AddStatusEffect(entity_t *entity, status_effect_type_t status_effect, s32 duration) {
    for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {

        s32 status_effect_type_value = entity->status_effects[i].type;

        if (entity->status_effects[i].type == status_effect_none) {
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
            TakeHP(entity, 25);
            break;
        case trap_type_poison:
            AddStatusEffect(entity, status_effect_poison, 3);
            break;
        default:
            break;
    }
}

fn void ApplyTileEffects(map_t *map, entity_t *entity)
{
    tile_t *tile = GetTile(map, entity->p.x, entity->p.y);

    if (tile->trap_type != trap_type_none)
    {
        ApplyTrapEffects(tile, entity);
    }
    // todo: cover mechanics stuff
    //if (tile->cover_type != cover_type_none) {
    //  ApplyCoverBonus(entity);
    //}
}

fn void ProcessStatusEffects(entity_t *entity)
{
    for (int i = 0; i < MAX_STATUS_EFFECTS; ++i) {
        if (entity->status_effects[i].type != status_effect_none) {
            switch (entity->status_effects[i].type) {
                case status_effect_poison:
                    TakeHP(entity, 10);
                    break;
                default:
                    break;
            }

            entity->status_effects[i].remaining_turns--;

            if (entity->status_effects[i].remaining_turns == 0) {
                DebugLog("Changing status effect to none, applying value: %d", status_effect_none);
                entity->status_effects[i].type = status_effect_none;
            }
        }
    }
}

fn void PushEntity(game_state_t *state, v2s source, entity_t *target, u8 push_distance, s32 strength)
{
    v2s direction = Sub32(target->p, source);

    if (direction.x != 0) direction.x = (direction.x > 0) ? 1 : -1;
    if (direction.y != 0) direction.y = (direction.y > 0) ? 1 : -1;

    s32 damage_per_tile = strength / (target->size.x * target->size.y);
    s32 total_damage = 0;

    for (s32 i = 0; i < push_distance; ++i) { // todo: make strength affect push_distance somewhat
        v2s next_pos = Add32(target->p, direction); // Slime is moved through each tile on the way

        if (!MoveFitsWithSize(state, target, next_pos)) {
            TakeHP(target, (s16)damage_per_tile);
            break;
        } else {
            target->p = next_pos;
            ApplyTileEffects(state->map, target);
        }
    }
}

fn s32 CalculateHitChance(const entity_t *user, const entity_t *target, action_type_t action_type)
{
    s32 final_hit_chance;
    f32 distance = DistanceV2S(user->p, target->p);

    if (action_type != action_melee_attack) {
        final_hit_chance = user->ranged_accuracy - target->evasion + BASE_HIT_CHANCE;
        if (user->has_hitchance_boost) {
            final_hit_chance = (s32)((f32)final_hit_chance * user->hitchance_boost_multiplier);
        }

        if (distance > MAX_EFFECTIVE_RANGE) {
            s32 penalty = (s32) (distance - MAX_EFFECTIVE_RANGE) * DISTANCE_PENALTY_PER_TILE;
            final_hit_chance -= penalty;
        }

    } else {
        final_hit_chance = user->ranged_accuracy - target->evasion + BASE_HIT_CHANCE + MELEE_BONUS;
        if (user->has_hitchance_boost) {
            final_hit_chance = (s32)((f32)final_hit_chance * user->hitchance_boost_multiplier);
        }
    }

    final_hit_chance = Clamp32(final_hit_chance, 0, 100);

    final_hit_chance = 100;
    return final_hit_chance;
}

typedef enum
{
    high_velocity,
    low_velocity
} hit_velocity_t;

fn void AddBloodOnNearbyTiles(map_t *map, v2s shooter_position, v2s hit_position,
                              blood_type_t blood_type, hit_velocity_t hit_velocity) {
    tile_t *initial_tile = GetTile(map, hit_position.x, hit_position.y);
    if (initial_tile && IsTraversable(map, hit_position)) {
        initial_tile->blood = blood_type;
    }

    v2s extended_position = Add32(hit_position, Sub32(hit_position, shooter_position));
    dda_line_t dda = BeginDDALine(map, hit_position, extended_position);

    int splatter_length;
    if (hit_velocity == high_velocity) {
        splatter_length = (rand() % 3) + 1;
    } else {
        splatter_length = 0;
    }
    int trail_count = 0;

    while(ContinueDDALine(&dda) && trail_count < splatter_length) {
        if (IsTraversable(map, dda.at)) {
            tile_t *tile = GetTile(map, dda.at.x, dda.at.y);
            if (tile) {
                tile->blood = blood_type;
                trail_count++;
            }
        } else {
            break;
        }
    }

        if (hit_velocity == low_velocity) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (rand() % 100 < 94) {
                    continue;
                }
                v2s splatter_pos = V2S(hit_position.x + dx, hit_position.y + dy);
                if (IsTraversable(map, splatter_pos)) {
                    tile_t *tile = GetTile(map, splatter_pos.x, splatter_pos.y);
                    if (tile) {
                        tile->blood = blood_type;
                    }
                }
            }
        }
    }
}

#define BLOOD_SPLATTER_CHANCE 0.3f
fn void DoDamage(game_state_t *game, entity_t *user, entity_t *target, s32 damage, const char *damage_type_prefix)
{
    damage = damage + (rand() % 3);
    LogLn(game->log, "%shit! inflicted %i %s of %sdamage upon the target!",
        damage_type_prefix, damage, damage == 1 ? "point" : "points", damage_type_prefix);
    TakeHP(target, (s16)damage);
    CreateDamageNumber(game->particles, Add(target->deferred_p, V2(-25.0f, -25.0f)), damage);

    if ((rand() / (float)RAND_MAX) < 20) {
        blood_type_t blood_type = (target->enemy_type == 0) ? blood_red : blood_green;
        hit_velocity_t hit_velocity = (rand() % 2 == 0) ? high_velocity : low_velocity; // Just a temporary thing until we add ammo types
        AddBloodOnNearbyTiles(game->map, user->p, target->p, blood_type, hit_velocity);
    }
}

fn inline void AOE(game_state_t *state, entity_t *user, entity_t *target, const action_params_t *params)
{
    entity_storage_t *storage = state->storage;
    
    const char *prefix = "blast ";
    s32 damage = params->damage;
    v2s area = params->area;
    s32 radius_inner = area.x;
    s32 radius_outer = area.x * (s32)2;
    v2s explosion_center = state->cursor->p;
    for (s32 i = 0; i < storage->EntityCount; i++)
    {
        entity_t *entity = &storage->entities[i];
        f32 distance = DistanceV2S(explosion_center, entity->p);
        
        if (distance <= radius_inner) {
            DoDamage(state, user, entity, damage, prefix);
        } else if (distance <= radius_outer) {
            DoDamage(state, user, entity, damage, prefix);
            PushEntity(state, explosion_center, entity, 2, 25);
        }
    }
}

fn inline void SingleTarget(game_state_t *state, entity_t *user, entity_t *target, const action_params_t *params)
{
    if ((IsPlayer(user) && state->turns->god_mode_enabled))
    {
        DoDamage(state, user, target, target->health, "");
    }
    else
    {
        s32 chance = CalculateHitChance(user, target, params->type);
        s32 roll = rand() % 100;
        s32 roll_crit = rand() % 100;

        b32 missed = !(roll < chance);
        b32 crited = (roll_crit < CRITICAL_DAMAGE_MULTIPLIER);
        b32 grazed = ((roll >= chance)) && (roll < (chance + GRAZE_THRESHOLD));

        if ((missed == false))
        {    
            if (crited) {
                DoDamage(state, user, target, params->damage * CRITICAL_DAMAGE_MULTIPLIER, "critical ");
                CreateCombatText(state->particles, target->deferred_p, 0);
            } else {
                DoDamage(state, user, target, params->damage, "");
                CreateCombatText(state->particles, target->deferred_p, 1);
            }
        }
        else
        {
            if (grazed) {
                DoDamage(state, user, target, params->damage / 2, "graze ");
                CreateCombatText(state->particles, target->deferred_p, 3);
            } else {
                LogLn(state->log, "missed!");
                CreateDamageNumber(state->particles, target->deferred_p, 0);
                CreateCombatText(state->particles, target->deferred_p, 2);
            }
        }
    }
}

// todo: In the future make walls protect entities from explosions using ray casting
fn void CommitAction(game_state_t *state, entity_t *user, entity_t *target, action_t *action, v2s target_p)
{
    const action_params_t *params = GetParameters(action->type);

    switch (params->mode)
    {
    case action_mode_damage:
    {
        if (IsZero(params->area))
        {
            SingleTarget(state, user, target, params);
        }
        else
        {
            AOE(state, user, target, params);
        }

        // NOTE(): Reset per-turn attack buffs/modifiers.
        user->has_hitchance_boost = false;
        user->hitchance_boost_multiplier = 1.0f;
    } break;
    case action_mode_heal: Heal(target, (s16) params->value); break;
    case action_mode_dash: user->p = target_p; break;
    }
}