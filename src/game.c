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

fn void InflictDamage(entity_t *entity, s16 damage) {
    if (entity == NULL) return;

    if (damage >= entity->health) {
        entity->health = 0;
        entity->flags |= entity_flags_deleted;
        // todo: handle entity death
    } else {
        entity->health -= damage;
    }
    entity->blink_time = 1.0f;
}

fn void Heal(entity_t *entity, s16 healed_hp) {
    if (entity == NULL) return;

    if (entity->health >= healed_hp) {
        entity->health = entity->max_health;
    } else {
        entity->health += healed_hp;
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

fn void ProcessStatusEffects(entity_t *entity)
{
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
            InflictDamage(target, (s16)damage_per_tile);
            break;
        } else {
            target->p = next_pos;
            ApplyTileEffects(next_pos, state, target);

        }
    }
}

fn s32 CalculateHitChance(entity_t *user, entity_t *target, action_type_t action_type)
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

    if (final_hit_chance > 100) {
        final_hit_chance = 100;
    }

    return final_hit_chance;
}

fn void HandleAttack(entity_t *user, entity_t *target, action_type_t action_type, game_world_t *state)
{
// todo: add critical hits, distance measure
    if (IsPlayer(user) && state->turns->god_mode_enabled)
    {
        InflictDamage(target, target->health);
    }
    else
    {
        s32 final_hit_chance = CalculateHitChance(user, target, action_type);
    
        if (final_hit_chance < 0) final_hit_chance = 0;
        if (final_hit_chance > 100) final_hit_chance = 100;
    
        s32 roll = rand() % 100;
        s32 crit_roll = rand() % 100;
    
        if (roll < final_hit_chance) {
    
            if (crit_roll < CRITICAL_DAMAGE_MULTIPLIER) {
                s32 crit_damage = user->attack_dmg * CRITICAL_DAMAGE_MULTIPLIER;
                PushEntity(state, user->p, target, 2, 25);
                InflictDamage(target, (u16)crit_damage);
                DebugLog("Critical Hit! Inflicted %i critical damage to target #%i", crit_damage, target->id);
            } else {
                InflictDamage(target, user->attack_dmg);
                DebugLog("Hit! Inflicted %i damage to target #%i", user->attack_dmg, target->id);
            }
    
        } else if (roll < final_hit_chance + GRAZE_THRESHOLD && roll >= final_hit_chance) {
            u16 graze_damage = user->attack_dmg / 2;
            InflictDamage(target, graze_damage);
            DebugLog("Grazing hit! Inflicted %i grazing damage to target #%i", graze_damage, target->id);
        } else {
            DebugLog("Missed! Ranged attack missed target #%i", target->id);
        }
    }
}

#define GRENADE_EXPLOSION_RADIUS 3  // temp value
#define GRENADE_DAMAGE 50           // temp value
// todo: In the future make walls protect entities from explosions
fn void ActivateSlotAction(game_world_t *state, entity_t *user, entity_t *target, action_t *action)
{
    entity_storage_t *storage = state->storage;
    turn_queue_t *queue = state->turns;
    if (ConsumeActionPoints(queue, queue->god_mode_enabled ? 0 : action->params.action_point_cost))
    {
        switch(action->type)
        {
            case action_ranged_attack:
            {
                HandleAttack(user, target, action->type, state);
                user->has_hitchance_boost = false;
                user->hitchance_boost_multiplier = 1.0f;
                break;
            }
            case action_melee_attack:
            {
                HandleAttack(user, target, action->type, state);
                user->has_hitchance_boost = false;
                user->hitchance_boost_multiplier = 1.0f;
                break;
            }
            case action_throw:
            {
                s32 radius_inner = action->params.area_of_effect.x;
                s32 radius_outer = action->params.area_of_effect.x * (s32)2;
                v2s explosion_center = state->cursor->p;


                for (s32 i = 0; i < storage->num; i++)
                {
                    entity_t *entity = &storage->entities[i];
                    f32 distance = DistanceV2S(explosion_center, entity->p);

                    if (distance <= radius_inner) {
                        InflictDamage(entity, (s16)action->params.damage * (s16)2.0f);
                    } else if (distance <= radius_outer) {
                        InflictDamage(entity, (s16)action->params.damage);

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
                    Heal(target, (s16)action->params.damage);
                    DebugLog("healed up for %i hp", (s16)action->params.damage);
                }
            }   break;
            case action_none:
            default:
                break;
        }
    }
}

fn void SubdivideLargeSlime(game_world_t *game, entity_t *entity, s32 x, s32 y)
{
    entity_t *result = CreateSlime(game, Add32(entity->p, V2S(x, y)));
    if (result)
        result->deferred_p = entity->deferred_p;
}