typedef struct
{
    v2 Min;
    v2 CellSize;

    command_buffer_t *Out;
    v2 At;
    bmfont_t *Font;
    interface_t *GUI;
    inventory_t *Eq;
} inventory_layout_t;

fn inventory_layout_t InventoryLayout(inventory_t *Eq, interface_t *GUI, command_buffer_t *Out, bmfont_t *Font, v2 At, v2 CellSize)
{
    inventory_layout_t result = {0};
    result.Eq = Eq;
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

fn bb_t GetItemBox(const inventory_layout_t *Layout, const item_t *Item)
{
    v2 ItemSz = SignedToFloat(Item->size);
    ItemSz = Mul(ItemSz, Layout->CellSize);
    return RectToBounds(V2(0.0f, 0.0f), ItemSz);
}

fn bb_t ItemBoxFromIndex(const inventory_layout_t *Layout, const item_t *Item, v2s Index)
{
    bb_t result = GetItemBox(Layout, Item);

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

    inventory_layout_t Layout = InventoryLayout(Eq, Interface, Out, Font, EqMin, CellSz);
    Layout.At.x += (GridSz.x + 10.0f);

    DrawRect(Out, EqMin, FullInventorySz, V4(0.0f, 0.0f, 0.0f, 0.7f)); // Background
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

            if ((Eq->layout[y][x].ID != 0))
                DrawRect(Out, Add(EqMin, relative), CellSz, Red());

            DrawRectOutline(Out, Add(EqMin, relative), CellSz, Green());
        }
    }

    // Items
    for (s32 index = 0; index < Eq->item_count; index++)
    {
        item_t *Item = &Eq->items[index];
        bb_t ItemBounds = ItemBoxFromIndex(&Layout, Item, V2S(Item->x, Item->y));

        // Interact
        if (IsPointInBounds(ItemBounds, Cursor) && (Interface->DraggedItemID == 0))
        {
            Tooltip = Item->params->name;
            if (Input->mouse_buttons[0])
            {
                const s32 FirstID = 0x100;
                Interface->DraggedItemID = FirstID + index;
                Interface->DraggedItem = *Item;
                Interface->OriginalX = Item->x;
                Interface->OriginalY = Item->y;
            }
        }

        // Render
        {
            const v4 ItemColors[4] = { Red(), Blue(), Yellow(), Green() };
            v4 Color = ItemColors[Item->params->type % ArraySize(ItemColors)];
            DrawBounds(Out, Shrink(ItemBounds, 5.0f), Color);
            DrawFormat(Out, Font, GetCenter(ItemBounds), White(), "#%i", Item->ID);
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
            bb_t ItemBounds = ItemBoxFromIndex(&Layout, &Interface->DraggedItem, Index);
            ItemBounds = Shrink(ItemBounds, 4.0f);
            DrawBoundsOutline(Out, ItemBounds, Red());
        }

        if (!Interface->Buttons[0]) // Place
        {
            Interface->DraggedItemID = 0;
            Eq_MoveItem(Eq, Interface->DraggedItem, Index);
        }
        if (Interface->Interact[1]) // Rotate
        {
            Interface->DraggedItem.size = RotateSigned(Interface->DraggedItem.size);
        }
    }
}