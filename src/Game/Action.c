fn action_t GetEquippedAction(const slot_bar_t *menu, entity_t *user)
{
    action_t result = { .type = action_none };
    s32 index = (menu->selected_slot - 1);
    if ((index >= 0) && (index < ArraySize(menu->slots)))
        result = menu->slots[index].action;
    return result;
}

fn inline void SetMenuShortcut(slot_bar_t *menu, assets_t *assets, s32 index,
                                action_type_t type, const item_params_t *item_params)
{
    action_t *shortcut = &(menu->slots[index].action);
#define Icons assets->CombatUI.action_bar_icons
    if (assets && (type < ArraySize(Icons)))
        shortcut->icon = &Icons[type][0];
    else
        shortcut->icon = NULL;
#undef Icons
    if (!item_params) {
        shortcut->params = GetParameters(type);
    } else shortcut->item_params = item_params;

    shortcut->type = type;
}

fn void DefaultActionBar(slot_bar_t *bar, assets_t *assets)
{
    bar->selected_slot = 1;

    for (s32 i = 0; i < ArraySize(bar->slots); i++)
    {
        bar->slots[i].action.type = action_none;
        bar->slots[i].action.icon = NULL;
    }

    s32 Index = 0;
    //SetMenuShortcut(bar, assets, Index++, action_melee_attack);
    //SetMenuShortcut(bar, assets, Index++, action_ranged_attack);
    //SetMenuShortcut(bar, assets, Index++, action_heal);
    //SetMenuShortcut(bar, assets, Index++, action_throw);
    //SetMenuShortcut(bar, assets, Index++, action_push);
    //SetMenuShortcut(bar, 0, Index++, action_slash);
    //SetMenuShortcut(bar, 0, Index++, action_dash);
}

fn inline s32 CalculateHitChance(const entity_t *user, const entity_t *target, action_type_t action_type)
{
    s32 final_hit_chance;
    f32 distance = IntDistance(user->p, target->p);

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

    final_hit_chance = Clamp(final_hit_chance, 0, 100);

    return final_hit_chance;
}