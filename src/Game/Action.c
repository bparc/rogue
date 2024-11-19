fn action_t GetEquippedAction(const slot_bar_t *menu, entity_t *user)
{
    action_t result = { .type = action_none };
    s32 index = (menu->selected_slot - 1);
    if ((index >= 0) && (index < ArraySize(menu->slots)))
        result = menu->slots[index].action;
    return result;
}

fn inline void SetMenuShortcut(slot_bar_t *menu, assets_t *assets, s32 index, action_type_t type)
{
    action_t *shortcut = &(menu->slots[index].action);
#define Icons assets->CombatUI.action_bar_icons
    if (assets && (type < ArraySize(Icons)))
        shortcut->icon = &Icons[type][0];
    else
        shortcut->icon = NULL;
#undef Icons
    shortcut->params = GetParameters(type);
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
    SetMenuShortcut(bar, assets, Index++, action_melee_attack);
    SetMenuShortcut(bar, assets, Index++, action_ranged_attack);
    SetMenuShortcut(bar, assets, Index++, action_heal);
    SetMenuShortcut(bar, assets, Index++, action_throw);
    SetMenuShortcut(bar, assets, Index++, action_push);
    SetMenuShortcut(bar, 0, Index++, action_slash);
    SetMenuShortcut(bar, 0, Index++, action_dash);
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

fn const char *NameFromActionType(const char *name, memory_t *memory)
{
    s32 length = StringLength(name);
    char *result = PushSize(memory, (length + 1));
    result[length] = 0;
    if (length)
        result[0] = ToUpper(result[0]);

    s32 at = 0;
    while (at < length)
    {
        char *ch = &result[at];
        *ch = ToUpper(name[at]);

        if (*ch == '_')
            *ch = ' ';

        at++;
    }

    return result;
}

void DefaultActionValues(void)
{
    for (s32 index = 0; index < ArraySize(_Global_Action_Data); index++)
    {
        action_params_t *Params = &_Global_Action_Data[index];
        Params->type = (action_type_t)index;
        if (Params->name == NULL)
            Params->name = _Global_Action_Names[index];
        if (!Params->range)
            Params->range = 2;

    }
}

fn void SetupActionDataTable(memory_t *memory, const assets_t *assets)
{
    #define ACT(Type) \
    _Global_Action_Names[action_##Type] = NameFromActionType(#Type, memory); \
    _Global_Action_Data[action_##Type]  = (action_params_t)

    ACT(melee_attack)
    {
        .damage = 3,
        .cost   = 1,
    };
    ACT(ranged_attack)
    {
        .range  = 5,
        .cost   = 1,
        .damage = 4,
    };
    ACT(heal)
    {
        .mode = action_mode_heal,
        .cost = 2,
        .damage = 10,
        .value  = 10,
        .target = target_self,
    };
    ACT(push)
    {
        .range = 2,
        .cost  = 3,
        .pushback = 2,
    };
    ACT(throw)
    {
        .animation_ranged = &assets->PlayerGrenade,
        .damage = 20,
        .range  = 6,
        .area   = {2, 2},
        .cost   = 1,
    };
    ACT(slash)
    {
        .damage = 9,
        .cost   = 3,
        .flags  = action_display_move_name,
    };
    ACT(dash)
    {
        .mode   = action_mode_dash,
        .flags  = action_display_move_name,
        .range  = 5,
        .cost   = 1,
    };
    ACT(slime_ranged)
    {
        .animation_ranged = &assets->SlimeBall,
        .range  = 5,
        .damage = 3,
    };
    #undef ACT

    DefaultActionValues();
}