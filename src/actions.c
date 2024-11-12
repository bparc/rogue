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