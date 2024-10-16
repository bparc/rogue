fn void ApplyTileEffects(v2s peekPos, game_world_t *state, entity_t *entity);
fn void ApplyTrapEffects(tile_t *tile, entity_t *entity);
fn void InflictDamage(entity_t *entity, u16 damage);
fn void ProcessStatusEffects(entity_t *entity);
fn void AddStatusEffect(entity_t *entity, status_effect_type_t status_effect, s32 duration);

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

fn void InflictDamage(entity_t *entity, u16 damage) {
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

fn void AddStatusEffect(entity_t *entity, status_effect_type_t status_effect, s32 duration) {
    for (int i = 0; i < MAX_STATUS_EFFECTS; ++i) {
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