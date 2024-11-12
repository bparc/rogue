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

fn void PlaceItemInInventory(inventory_t *Eq, interface_t *Interface,
                            v2s Index, v2 CellSz, v2 Offset, entity_t *User) {
    item_t *Item = &Interface->DraggedItem;
    item_t *NewItem = NULL;
    const item_params_t *Params = Item->params;

    v2 EqSz = {0};
    if (Interface->DraggedItemRotation == 0) { // No rotation
        EqSz.x = (f32)CellSz.x;
        EqSz.y = (f32)CellSz.y;
    } else { // 90 degrees rotation
        EqSz.x = (f32)CellSz.y;
        EqSz.y = (f32)CellSz.x;
    }
    EqSz = Mul(EqSz, CellSz);

    s32 new_x = Index.x;
    s32 new_y = Index.y;

    v2 At = {0};
    At.x = (f32)Index.x;
    At.y = (f32)Index.y;
    At = Mul(At, CellSz);
    At = Add(At, Offset);
    bb_t ItemBb = RectToBounds(At, CellSz);

    b32 itemsOverlap = false;
    for (int i = 0; i < Eq->item_count; i++) {
        v2 EqSz1 = {0};
        EqSz1.x = (f32)Eq->items[i].params->EqSize.x;
        EqSz1.y = (f32)Eq->items[i].params->EqSize.y;
        EqSz1 = Mul(EqSz1, CellSz);

        v2 At1 = {0};
        At1.x = (f32)Eq->items[i].x;
        At1.y = (f32)Eq->items[i].y;
        At1 = Mul(At1, CellSz);
        At1 = Add(At1, Offset);
        bb_t SecondItemBb = RectToBounds(At1, EqSz1);

        if (DoBoundingBoxesOverlap(ItemBb, SecondItemBb) && Item->params->id != Eq->items[i].params->id) {
            itemsOverlap = true;
            break;
        }
    }

    if (new_x >= 0 && new_y >= 0 &&
        new_x + Params->EqSize.x <= Eq->x &&
        new_y + Params->EqSize.y <= Eq->y) {
        if (!itemsOverlap && RemoveItemFromInventory(User, Params->id)) {
            NewItem = AddItemToInventory(User, Item->type);
            NewItem->x = Index.x;
            NewItem->y = Index.y;
            // NOTE(): Update the item rotation.
            NewItem->params->rotation = Interface->DraggedItemRotation;
        }
    }

    Interface->DraggedItemRotation = Item->params->rotation;
}

typedef struct
{
    v2 Min;
    v2 CellSize;

    command_buffer_t *Out;
    v2 At;
    bmfont_t *Font;
    interface_t *GUI;
} inventory_layout_t;

fn inventory_layout_t InventoryLayout(interface_t *GUI, command_buffer_t *Out, bmfont_t *Font, v2 At, v2 CellSize)
{
    inventory_layout_t result = {0};
    result.GUI = GUI;
    result.Font = Font;
    result.Out = Out;
    result.Min = result.At = At;
    result.CellSize = CellSize;

    return result;
}

fn void InventoryText(inventory_layout_t *Layout, const char *format, ...)
{
    char string[256] = "";
    va_list args = {0};
    va_start(args, format);
    vsnprintf(string, ArraySize(string), format, args);
    va_end(args);
    DrawText(Layout->Out, Layout->Font, Layout->At, string, White());
    Layout->At.y += 20.0f;
}

fn void InventorySeparator(inventory_layout_t *Layout)
{
    Layout->At.y += 20.0f;
}

fn v2 GetInventoryCellPosition(const inventory_layout_t *Layout, const item_t *Item, v2s Index)
{
    v2 result = {Item->x * Layout->CellSize.x, Item->y * Layout->CellSize.y};
    return result;
}

fn bb_t GetItemBox(const inventory_layout_t *Layout, const item_t *Item, b32 rotate)
{
    v2 ItemSz = SignedToFloat(Item->params->EqSize);
    if (rotate)
        ItemSz = Rotate(ItemSz);
    ItemSz = Mul(ItemSz, Layout->CellSize);
    return RectToBounds(V2(0.0f, 0.0f), ItemSz);
}

fn bb_t ItemBoxFromIndex(const inventory_layout_t *Layout, const item_t *Item, v2s Index, b32 rotate)
{
    bb_t result = GetItemBox(Layout, Item, rotate);

    v2 position = Mul(SignedToFloat(Index), Layout->CellSize);
    position = Add(position, Layout->Min);
    
    result.min = Add(result.min, position);
    result.max = Add(result.max, position);
    return result;
}

fn v2s InventoryCellFromOffset(const inventory_layout_t *Layout, v2 offset)
{
    v2 result = {0};
    result = Sub(offset, Layout->Min);
    result = Div(result, Layout->CellSize);
    return FloatToSigned(result);
}

fn void Inventory(command_buffer_t *Out, inventory_t *Eq, const client_input_t *Input, bmfont_t *Font, interface_t *Interface, entity_t *User)
{
    v2 Cursor = GetCursorOffset(Input);
    const char *Tooltip = NULL;

    // NOTE(): Layout:
    v2 EqMin = V2(100.0f, 40.0f);
    v2 CellSz = V2(32.0f, 32.0f);
    v2 GridSz = {Eq->x * CellSz.x, Eq->y * CellSz.y};
    v2 FullInventorySz = {GridSz.x + 180.0f, GridSz.y};

    inventory_layout_t Layout = InventoryLayout(Interface, Out, Font, EqMin, CellSz);
    Layout.At.x += (GridSz.x + 10.0f);

    DrawRect(Out, EqMin, FullInventorySz, V4(0.0f, 0.0f, 0.0f, 0.7f));
    InventoryText(&Layout, "i - close");
    InventoryText(&Layout, "Weight: %i/%i", Eq->carried_weight, Eq->max_carry_weight);
    InventorySeparator(&Layout);

    InventoryText(&Layout, "Health: %i/%i", User->health, User->max_health);
    InventoryText(&Layout, "Attack: %d", User->attack_dmg);
    InventoryText(&Layout, "Ranged Accuracy: %d", User->ranged_accuracy);
    InventoryText(&Layout, "Melee Accuracy: %d", User->melee_accuracy);
    InventoryText(&Layout, "Evasion: %d", User->evasion);
    InventoryText(&Layout, "Action Points: %d", User->remaining_action_points);

    // Grid
    for (s32 y = 0; y < Eq->y; y++)
    {
        for (s32 x = 0; x < Eq->x; x++)
        {   
            v2 relative = {x * CellSz.x, y * CellSz.y};
            DrawRectOutline(Out, Add(EqMin, relative), CellSz, Red());
        }
    }

    // Items
    for (s32 index = 0; index < Eq->item_count; index++)
    {
        item_t *Item = &Eq->items[index];
        bb_t ItemBounds = ItemBoxFromIndex(&Layout, Item, V2S(Item->x, Item->y), Item->params->rotation);

        // Interact
        if (IsPointInBounds(ItemBounds, Cursor) && (Interface->DraggedItemID == 0))
        {
            Tooltip = Item->params->name;
            if (Input->mouse_buttons[0])
            {
                const s32 FirstID = 0x100;
                Interface->DraggedItemID = FirstID + index;
                Interface->DraggedItemSz = SignedToFloat(Item->params->EqSize);
                Interface->DraggedItemIndex = index;
                Interface->DraggedItem = *Item;
                Interface->OriginalX = Item->x;
                Interface->OriginalY = Item->y;

                Interface->DraggedItemRotation = Item->params->rotation;
            }
        }

        // Render
        {
            const v4 ItemColors[4] = { Red(), Blue(), Yellow(), Green() };
            v4 Color = ItemColors[Item->type % ArraySize(ItemColors)];
            DrawBounds(Out, Shrink(ItemBounds, 5.0f), Color);
        }
    }

    if (Tooltip)
        DrawText(Out, Font, Add(Cursor, V2(15,23)), Tooltip, Yellow());

    // Dragging
    if (Interface->DraggedItemID)
    {
        v2s Index = InventoryCellFromOffset(&Layout, Cursor);

        // Render
        {
            bb_t ItemBounds = ItemBoxFromIndex(&Layout, &Interface->DraggedItem, Index, Interface->DraggedItemRotation);
            ItemBounds = Shrink(ItemBounds, 4.0f);
            DrawBounds(Out, ItemBounds, W(Green(), 0.5f));
        }

        if (!Interface->Buttons[0]) // Place
        {
            Interface->DraggedItemID = 0;
            PlaceItemInInventory(Eq, Interface, Index, Interface->DraggedItemSz, EqMin, User);

        }
        if (Interface->Interact[1]) // Rotate
        {
            // TODO(mw00): use rotation variable in item_t, instead of interface_t,
            // because it affects all items in inventory
            //Interface->DraggedItem.params->rotation = 1 - Interface->DraggedItemRotation;
            Interface->DraggedItemSz = Rotate(Interface->DraggedItemSz);
            Interface->DraggedItemRotation = (1 - Interface->DraggedItemRotation);
        }
    }
}

fn void HUD(command_buffer_t *out,game_world_t *state, turn_queue_t *queue, entity_storage_t *storage, assets_t *assets, const client_input_t *input)
{
    BeginInterface(state->interface, input);

    entity_t *ActiveEntity = GetActiveUnit(queue);

    TurnQueue(out, state, queue, assets, state->cursor);

    if (IsPlayer(ActiveEntity) && (state->interface->inventory_visible))
    {
        Inventory(out, ActiveEntity->inventory, input, assets->Font, state->interface, ActiveEntity);
        ActionMenu(ActiveEntity, state, out, assets, input, queue);
    }

    EndInterface(state->interface);
}