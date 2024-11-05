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

fn void ActionMenu(entity_t *user, game_world_t *state, command_buffer_t *out, assets_t *assets, const client_input_t *input, turn_queue_t *queue) {
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

    slot_bar_t *Bar = &state->slot_bar;

    for (s32 i = 0; i < ArraySize(Bar->slots); i++)
    {
        slot_t *slot = &Bar->slots[i];
        action_t *action = &slot->action;
        const action_params_t *params = GetParameters(action->type);

        v2 slot_offset = V2(i * (slot_size.x + padding), 0.0f);
        v2 slot_p = Add(slot_start_pos, slot_offset);

        v4 border_color = (Bar->selected_slot == (i + 1)) ? Blue() : Black();
        DrawRectOutline(out, slot_p, slot_size, border_color);

        if (action->icon)
        {
            v4 color = PureWhite();

            if (queue->action_points < GetAPCost(action->type))
                color = RGB(30, 30, 30);
            DrawBitmap(out, slot_p, slot_size, color, action->icon);
        }
        else
        {
            if (params->name)
                DrawText(out, assets->Font, Add(slot_p, V2(4, 2)), params->name, White());
        }

        if (IsKeyPressed(input, key_code_1 + (u8)i))
            Bar->selected_slot = i + 1;
    }
}

fn void DrawTurnQueuePic(command_buffer_t *out, f32 x, f32 y, v2 sz, f32 alpha, entity_t *entity, assets_t *assets)
{
    bitmap_t *bitmap = IsHostile(entity) ? &assets->Slime : &assets->Player[0];         
    v2 p = V2(x, y);

    v4 bitmap_color = PureWhite();
    bitmap_color.w = alpha;
    
    DrawBitmap(out, p, sz, bitmap_color, &assets->CombatUI.action_bar_elements[0]);
    if (bitmap)
        DrawBitmap(out, Add(p, V2(0.0f, 5.0f)), sz, bitmap_color, bitmap);
}

fn void DrawTurnQueuePicFadeOut(command_buffer_t *out, f32 x, f32 *y, v2 sz, entity_t *entity, assets_t *assets, f32 t, f32 y_spacing)
{
    t = t * t;

    v4 color = Black();
    color.w = t;
    x += (sz.x * (1.0f - t)); 
    DrawTurnQueuePic(out, -x, *y, sz, color.w, entity, assets);
    *y += y_spacing * t;
}

fn void TurnQueue(command_buffer_t *out, game_world_t *state, turn_queue_t *queue, assets_t *assets, cursor_t *cursor)
{
    entity_storage_t *storage = queue->storage;

    v2 sz = V2(64.f, 64.f);
    f32 spacing = sz.y + 5.0f;
    f32 top = 100.0f;
    f32 min = 8.0f;

    f32 x = min;
    f32 y = top;

    f32 fade_out_time = 0.5f;
    if ((queue->prev_turn_entity > 0) &&
        (queue->seconds_elapsed < fade_out_time))
    {
        f32 t = 1.0f - (queue->seconds_elapsed / fade_out_time);
        entity_t *entity = GetEntity(storage, queue->prev_turn_entity);
        DrawTurnQueuePicFadeOut(out, x, &y, sz, entity, assets, t, spacing);
    }

    for (s32 index = queue->num - 1; index >= 0; index--)
    {
        entity_id_t ID = queue->entities[index];
        entity_t *entity = GetEntity(storage, ID);
        if (entity)
        {
            DrawTurnQueuePic(out, x, y, sz, 1.0f, entity, assets);
            if (entity->id == cursor->Target)
                DrawRectOutline(out, V2(x, y), sz, Orange());

            y += spacing;
        }
        else
        {
            evicted_entity_t *evicted = GetEvictedEntity(queue, ID);
            if (evicted)
                DrawTurnQueuePicFadeOut(out, x, &y, sz, &evicted->entity, assets, evicted->time_remaining, spacing);
        }
    }

    DrawRectOutline(out, V2(x, 100.0f), sz, Red());
}

fn void RenderHP(command_buffer_t *out, v2 p, assets_t *assets, entity_t *entity)
{
    // todo: Add animation when chunk of health is lost, add art asset
    f32 health_percentage = (f32)entity->health / entity->max_health;

    v2 bar_size = V2(16*4, 2*4);
    v2 bar_p = p;

    DrawRect(out, bar_p, bar_size, Black());
    v2 health_bar_size = V2(bar_size.x * health_percentage, bar_size.y);
            
    DrawRect(out, bar_p, health_bar_size, Green());
    DrawRectOutline(out, bar_p, health_bar_size, Black());
    DrawFormat(out, assets->Font, Add(bar_p, V2(4.0f,-10.0f)), White(), "%i/%i", entity->health,entity->max_health);
}

fn void Inventory(command_buffer_t *Out, inventory_t *Eq, const client_input_t *Input, bmfont_t *Font, interface_t *Interface, entity_t *User)
{
    v2 Offset = V2(100.0f, 40.0f);
    v2 CellSz = V2(32.0f, 32.0f);

    v2 Cursor = GetCursorP(Input);
    const char *Tooltip = NULL;

    // Grid

    for (s32 y = 0; y < Eq->y; y++)
    {
        for (s32 x = 0; x < Eq->x; x++)
        {   
            v2 At = {0};
            At.x = (f32)x;
            At.y = (f32)y;
            At = Mul(At, CellSz);
            At = Add(At, Offset);
            //DrawRect(Out, At, CellSz, Black());
            DrawRectOutline(Out, At, CellSz, Red());

            const layout_cell_t *Cell = &Eq->layout[y][x];
            if (Cell->ID > 0)
            {
                bb_t CellBb = RectToBounds(At, CellSz);
                CellBb = Shrink(CellBb, 2.0f);
                //DrawRect(Out, CellBb.min, Sub(CellBb.max, CellBb.min), Green());
            }
        }
    }

    // Items

    for (s32 index = 0; index < Eq->item_count; index++)
    {
        item_t *Item = &Eq->items[index];
        const item_params_t *Params = Item->params;

        v2s Index = {0};
        Index.x = Item->x;
        Index.y = Item->y;

        v2 At = {0};
        At.x = (f32)Item->x;
        At.y = (f32)Item->y;
        At = Mul(At, CellSz);
        At = Add(At, Offset);

        v2 EqSz = {0};
        EqSz.x = (f32)Params->EqSize.x;
        EqSz.y = (f32)Params->EqSize.y;
        EqSz = Mul(EqSz, CellSz);

        v4 Colors[4] = { Red(), Blue(), Yellow(), Green() };
        v4 Color = Colors[Item->type % ArraySize(Colors)];

        bb_t ItemBb = RectToBounds(At, EqSz);

        bb_t BitmapBb = Shrink(ItemBb, 5.0f);
        DrawRect(Out, BitmapBb.min, Sub(BitmapBb.max, BitmapBb.min), Color);

        if (IsPointInBounds(ItemBb, Cursor) && (Interface->Locked == 0))
        {
            Tooltip = Params->name;
            if (Input->mouse_buttons[0])
            {
                Interface->Locked = (0x100 + index);
                Interface->DraggedItemSz = Params->EqSize;
                Interface->DraggedItemIndex = index;
                Interface->DraggedItem = *Item;
            }
        }
    }

    if (Tooltip)
        DrawText(Out, Font, Cursor, Tooltip, Yellow());

    // Dragging
    if (Interface->Locked)
    {
        v2 Rel = Sub(Cursor, Offset);
        Rel = Div(Rel, CellSz);

        v2s Index = {0};
        Index.x = (s32)Rel.x;
        Index.y = (s32)Rel.y;

        if (!Interface->Buttons[0]) // Place
        {
            Interface->Locked = 0;
            //item_t *Item = &Interface->DraggedItem;
            //item_t *NewItem = NULL;
            //RemoveItemFromInventory(User, Interface->DraggedItemIndex);
            //NewItem = AddItemToInventory(User, Item->type);
            //NewItem->x = Index.x;
            //NewItem->y = Index.y;
        }
        if (Interface->Interact[1]) // Rotate
        {
            v2s RotatedSz = {0};
            RotatedSz.y = Interface->DraggedItemSz.x;
            RotatedSz.x = Interface->DraggedItemSz.y;

            Interface->DraggedItemSz = RotatedSz;
        }

        v2 EqSz = {0};
        EqSz.x = (f32)Interface->DraggedItemSz.x;
        EqSz.y = (f32)Interface->DraggedItemSz.y;
        EqSz = Mul(EqSz, CellSz);

        v2 At = {0};
        At.x = (f32)Index.x;
        At.y = (f32)Index.y;
        At = Mul(At, CellSz);
        At = Add(At, Offset);
        bb_t ItemBb = RectToBounds(At, EqSz);
        ItemBb = Shrink(ItemBb, 4.0f);

        DrawRectOutline(Out, ItemBb.min, Sub(ItemBb.max, ItemBb.min), Green());
    }
}

fn void HUD(command_buffer_t *out,game_world_t *state, turn_queue_t *queue, entity_storage_t *storage, assets_t *assets, const client_input_t *input)
{
    BeginInterface(state->interface, input);

    entity_t *ActiveEntity = GetActiveUnit(queue);

    TurnQueue(out, state, queue, assets, state->cursor);

    if (IsPlayer(ActiveEntity))
    {
        Inventory(out, ActiveEntity->inventory, input, assets->Font, state->interface, ActiveEntity);
        ActionMenu(ActiveEntity, state, out, assets, input, queue);
    }

    EndInterface(state->interface);
}