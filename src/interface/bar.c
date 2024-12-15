fn inline void SetMenuShortcut(slot_bar_t *menu, assets_t *assets, s32 index,
                                action_type_t type, const item_params_t *item_params)
{
    Assert(0);
    #if 0
    action_t *shortcut = &(menu->slots[index].action);
#define Icons assets->CombatUI.action_bar_icons
    if (assets && (type < ArraySize(Icons)))
        shortcut->icon = &Icons[type][0];
    else
        shortcut->icon = NULL;
#undef Icons
    if (!item_params) {
        shortcut->params = GetActionParams(type);
    } else shortcut->item_params = item_params;

    shortcut->type = type;
    #endif
}

fn action_t GetEquippedAction(const slot_bar_t *menu, entity_t *user)
{
    action_t Result = ActionFromType(action_none);

    const slot_t *Slot = GetSlot((slot_bar_t *)menu, (s32)(menu->selected_slot - 1));
    if (Slot)
    {
        item_t *Item = Eq_GetItem(user->inventory, Slot->AssignedItem);
        if (Item)
        {
            Result = ActionFromType(Item->params->action);
        }
    }
    return Result;
}

fn void SetupActionBar(slot_bar_t *bar, assets_t *assets)
{
    bar->selected_slot = 1;

    for (s32 i = 0; i < ArraySize(bar->slots); i++)
    {
        bar->slots[i].action = ActionFromType(action_none);
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

fn slot_t *GetSlot(slot_bar_t *Bar, s32 Index)
{
    slot_t *Result = 0;
    if ((Index >= 0) && (Index < ArraySize(Bar->slots)))
    {
        Result = &Bar->slots[Index];
    }
    return Result;
}

fn inline void AssignItem(slot_bar_t *Bar, item_id_t ItemID, s8 Index)
{
    slot_t *Slot = GetSlot(Bar, Index);
    if (Slot)
    {
        Slot->AssignedItem = ItemID;
        DebugLog("assigning item to slot %i", Index);
    }
}

fn void ActionMenu(entity_t *user, game_state_t *state, command_buffer_t *out, assets_t *assets,
                    const client_input_t *input, game_state_t *queue, interface_t *In, entity_t *User) {
    v2 action_bar_size = V2(540.0f, 60.0f);
    v2 slot_size = V2(50.0f, 50.0f);
    
    f32 padding = 10.0f;
    f32 total_width = 9 * slot_size.x + 8 * padding;

    f32 screen_width = input->viewport[0];
    f32 screen_height = input->viewport[1];
    f32 start_pos_x = (screen_width - action_bar_size.x) / 2.0f; // 530 pikseli
    f32 start_pos_y = screen_height - action_bar_size.y - 10.0f; // 830 pikseli
    v2 action_bar_pos = V2(start_pos_x, start_pos_y); // (530, 830)

    DrawBitmap(out, action_bar_pos, action_bar_size, PureWhite(), &assets->CombatUI.action_bar_full);

    v2 slot_start_pos = Add(action_bar_pos, V2(5.0f, 5.0f)); // (535, 835)

    slot_bar_t *Bar = &state->Bar;

    const char *TooltipText = NULL;

    for (s32 i = 0; i < ArraySize(Bar->slots); i++)
    {
        slot_t *slot = &Bar->slots[i];
        action_t *action = &slot->action;

        v2 slot_offset = V2(i * (slot_size.x + padding), 0.0f);
        v2 slot_p = Add(slot_start_pos, slot_offset);

        v4 border_color = (Bar->selected_slot == (i + 1)) ? Blue() : Black();
        DrawRectOutline(out, slot_p, slot_size, border_color);

        #if 0
        if (action->icon)
        {
            v4 color = PureWhite();

            if (queue->action_points < GetAPCost(action->type))
                color = RGB(30, 30, 30);
            DrawBitmap(out, slot_p, slot_size, color, action->icon);
        }
        else
        {
            if (action_params && action_params->name && action_params->type != action_none) {
                DrawText(out, assets->Font, Add(slot_p, V2(4, 2)), action_params->name, White());
            }
            if (action_params->type == action_none && item_params && item_params->name) {
                DrawText(out, assets->Font, Add(slot_p, V2(4, 2)), item_params->name, White());
            }
        }
        #endif

        const char *Tooltip = "";
        item_t *Item = Eq_GetItem(User->inventory, slot->AssignedItem);
        if (Item)
        {
            DrawRect(out, slot_p, slot_size, Red());
            Tooltip = Item->params->name;
        }

        if (BoundsContains(RectBounds(slot_p, slot_size), In->Cursor))
        {
            TooltipText = Tooltip;

            if (In->DraggedItemID)
            {
                DrawRectOutline(out, slot_p, slot_size, Yellow());

                if (!In->Buttons[0])
                {
                    AssignItem(Bar, In->DraggedItemID, (s8)i);
                    In->DraggedItemID = 0;
                }
            }
        }

        if (IsKeyPressed(input, key_code_1 + (u8)i))
            Bar->selected_slot = i + 1;
    }

    if (TooltipText)
    {
        DrawText(out, assets->Font, GetCursorOffset(input), TooltipText, White());
    }
}