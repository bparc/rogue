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

fn void PushEntity(game_world_t *state, v2s source, entity_t *target, u8 push_distance, s32 strength)
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
            ApplyTileEffects(next_pos, state, target);

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
fn void DoDamage(game_world_t *game, entity_t *user, entity_t *target, s32 damage, const char *damage_type_prefix)
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

fn void HandleAttack(game_world_t *state, entity_t *user, entity_t *target, action_type_t action_type)
{
    if ((IsPlayer(user) && state->turns->god_mode_enabled))
    {
        DoDamage(state, user, target, target->health, "");
    }
    else
    {
        s32 chance = CalculateHitChance(user, target, action_type);
        s32 roll = rand() % 100;
        s32 roll_crit = rand() % 100;

        b32 missed = !(roll < chance);
        b32 crited = (roll_crit < CRITICAL_DAMAGE_MULTIPLIER);
        b32 grazed = ((roll >= chance)) && (roll < (chance + GRAZE_THRESHOLD));

        if ((missed == false))
        {    
            if (crited)
                DoDamage(state, user, target, user->attack_dmg * CRITICAL_DAMAGE_MULTIPLIER, "critical ");
            else
                DoDamage(state, user, target, user->attack_dmg, "");
        }
        else
        {
            if (grazed)
                DoDamage(state, user, target, user->attack_dmg / 2, "graze ");
            else
            {
                LogLn(state->log, "missed!");
                CreateDamageNumber(state->particles, target->deferred_p, 0);
            }
        }
    }
}

#define GRENADE_EXPLOSION_RADIUS 3  // temp value
#define GRENADE_DAMAGE 50           // temp value
// todo: In the future make walls protect entities from explosions using ray casting
fn void CommitAction(game_world_t *state, entity_t *user, entity_t *target, action_t *action, v2s target_p)
{
    const action_params_t *Params = GetParameters(action->type);

    entity_storage_t *storage = state->storage;
    turn_queue_t *queue = state->turns;

    switch(action->type)
    {
        case action_ranged_attack:
        {
            HandleAttack(state, user, target, action->type);
            user->has_hitchance_boost = false;
            user->hitchance_boost_multiplier = 1.0f;
            break;
        }
        case action_melee_attack:
        {
            HandleAttack(state, user, target, action->type);
            user->has_hitchance_boost = false;
            user->hitchance_boost_multiplier = 1.0f;
            break;
        }
        case action_throw:
        {
            const char *prefix = "blast ";
            s32 damage = Params->damage;
            v2s area = Params->area;
            s32 radius_inner = area.x;
            s32 radius_outer = area.x * (s32)2;
            v2s explosion_center = state->cursor->p;
            for (s32 i = 0; i < storage->num; i++)
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
            break;
        }
        case action_push:
        {
            PushEntity(state, user->p, target, 4, 25);
            break;
        }
        case action_heal_self:
        {
            if (target == user)
            {
                Heal(target, (s16) Params->value);
                DebugLog("healed up for %i hp", (s16)Params->value);
            }
        }   break;
        case action_none:
        default:
            break;
    }
}

fn void SubdivideLargeSlime(game_world_t *game, entity_t *entity, s32 x, s32 y)
{
    entity_t *result = CreateSlime(game, Add32(entity->p, V2S(x, y)));
    if (result)
        result->deferred_p = entity->deferred_p;
}

fn v2 CameraToScreen(const game_world_t *game, v2 p)
{
    p = ScreenToIso(p);
    p = Add(p, game->camera_position);
    p = Scale(p, VIEWPORT_INTEGER_SCALE);
    return p;
}