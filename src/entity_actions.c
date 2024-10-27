fn action_t GetEquippedAction(const slot_bar_t *menu, entity_t *user)
{
    action_t result = { .type = action_none };
    s32 index = (menu->selected_slot - 1);
    if ((index >= 0) && (index < ArraySize(menu->slots)))
        result = menu->slots[index].action;
    return result;
}