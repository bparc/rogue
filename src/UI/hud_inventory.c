typedef struct
{
    v2 Min;
    v2 CellSize;

    v2 ContextMenuMin;
    s32 ContextMenuItemCount;
    f32 ContextMenuItemWidth;
    f32 LineHeight;

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
    result.LineHeight = 20.0f;
    result.ContextMenuItemWidth = 100.0f;

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
    Layout->At.y += Layout->LineHeight;
}

fn void InventorySeparator(inventory_layout_t *Layout)
{
    Layout->At.y += Layout->LineHeight;
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
    offset = Sub(offset, Layout->Min);
    offset = Div(offset, Layout->CellSize);

    v2s result = FloatToSigned(offset);
    return result;
}

fn b32 ContextMenuItem(inventory_layout_t *Layout, const char *Text)
{
    b32 Result = 0;
    bb_t Bounds = RectToBounds(Layout->At, V2(Layout->ContextMenuItemWidth, Layout->LineHeight));

    v4 Background = Red();
    if (IsPointInBounds(Bounds, Layout->GUI->Cursor) && (!Layout->GUI->CloseContextMenu))
    {
        Background = Pink();
        if (Layout->GUI->Interact[0])
        {
            Result = true;
            Layout->GUI->CloseContextMenu = true;
        }
    }

    f32 T = Layout->GUI->ContextMenuT;
    DrawBounds(Layout->Out, Bounds, W(Background, T));

    {
        v4 TextColor = White();
        v2 TextMin = Bounds.min;
        TextColor.w = T;
        TextMin.x += 4.0f;

        TextMin.y = Lerp(Layout->ContextMenuMin.y, TextMin.y, T * T);

        DrawText(Layout->Out, Layout->Font, TextMin, Text, TextColor);
    }

    Layout->At.y += Layout->LineHeight;
    Layout->ContextMenuItemCount++;

    return Result;
}

fn void ContextMenu(interface_t *In, inventory_layout_t Layout, command_buffer_t *Out, entity_t *User, inventory_t *Eq, item_id_t ItemID, turn_system_t *TurnSystem)
{
    item_t *Item = Eq_GetItem(Eq, ItemID);
    if (Item)
    {
        if (Item->params->action != action_none)
        {
            if (ContextMenuItem(&Layout, "Use"))
                UseItem(TurnSystem, User, Eq, *Item);
        }

        if (ContextMenuItem(&Layout, "Examine"))
            DebugLog("Examine...");
        if (ContextMenuItem(&Layout, "Remove"))
            Eq_RemoveItem(Eq, ItemID);
        ContextMenuItem(&Layout, "Drop");
        ContextMenuItem(&Layout, "Exit");
    }
}

fn void OpenContainer(interface_t *In, container_t *Container)
{
    if (Container)
    {
        In->OpenedContainer = Container;
    }
}

fn void CloseContainer(interface_t *In)
{
    if (In->OpenedContainer != NULL)
    {
        In->OpenedContainer = NULL;
        CloseInventory(In);
    }
}

fn void Inventory(interface_t *In, v2 EqMin, inventory_t *Eq, entity_t *User, container_t *Container, b32 DrawUserInfo)
{
    command_buffer_t *Out = In->Out;
    const virtual_controls_t *Cons = In->Cons;
    f32 dt = In->DeltaTime;
    turn_system_t *TurnSystem = In->TurnSystem;
    bmfont_t *Font = In->Font;
    const client_input_t *Input = In->Input;

    v2 Cursor = GetCursorOffset(Input);
    const char *Tooltip = NULL;

    // NOTE(): Layout:
    v2 CellSz = {32.0f, 32.0f};
    v2 GridSz = {Eq->x * CellSz.x, Eq->y * CellSz.y};
    v2 FullInventorySz = {GridSz.x, GridSz.y};
    bb_t GridBounds = RectToBounds(EqMin, GridSz);
    if (DrawUserInfo)
        FullInventorySz.x += 180.0f;

    inventory_layout_t Layout = InventoryLayout(Eq, In, Out, Font, EqMin, CellSz);
    Layout.At.x += (GridSz.x + 10.0f);

    DrawRect(Out, EqMin, FullInventorySz, V4(0.0f, 0.0f, 0.0f, 0.7f)); // Background

    if (DrawUserInfo)
    {
        InventoryText(&Layout, "TAB - close");
        InventoryText(&Layout, "Weight: %i/%i", Eq->carried_weight, Eq->max_carry_weight);
        InventorySeparator(&Layout);
    
        InventoryText(&Layout, "Health: %i/%i", User->health, User->max_health);
        InventoryText(&Layout, "Attack: %d", User->attack_dmg);
        InventoryText(&Layout, "Ranged Accuracy: %d", User->ranged_accuracy);
        InventoryText(&Layout, "Melee Accuracy: %d", User->melee_accuracy);
        InventoryText(&Layout, "Evasion: %d", User->evasion);
        InventoryText(&Layout, "Action Points: %d", User->remaining_action_points);
    }

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
        b32 NoWidgetActive = (In->DraggedItemID == 0) && (!In->ContextMenuOpened);
        if (NoWidgetActive &&
            (IsPointInBounds(ItemBounds, Cursor)) &&
            (IsPointInBounds(GridBounds, Cursor)))
        {
            Tooltip = Item->params->name;
            if (In->Interact[0])
                BeginItemDrag(In, Item, Eq);

            if ((In->DraggedItemID == 0) && In->Interact[1])
                OpenContextMenu(In, Item->ID);
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
    if (In->DraggedItemID && (IsPointInBounds(GridBounds, Cursor)))
    {
        v2s Index = InventoryCellFromOffset(&Layout, Cursor);

        {
            v2s MaxIndex = Sub32(Eq->size, In->DraggedItem.size);
            Index = ClampVector32(Index, V2S(0, 0), MaxIndex);
        }

        // Render
        {
            item_t *DraggedItem = &In->DraggedItem;
            bb_t ItemBounds = ItemBoxFromIndex(&Layout, DraggedItem, Index);
            b32 CanBePlaced = Eq_IsSpaceFree_Exclude(Eq, Index, DraggedItem->size, DraggedItem->ID);
            DrawBoundsOutline(Out, Shrink(ItemBounds, 4.0f), CanBePlaced ? Blue() : Red());
        }

        if (!In->Buttons[0]) // Place
        {
            inventory_t *Dest = Eq;
            inventory_t *Source = In->DraggedContainer;

            if (Dest == Source)
                Eq_MoveItem(Source, In->DraggedItem, Index);
            else
                Eq_TransferItem(Source, Dest, In->DraggedItem, Index);

            In->DraggedItemID = 0;

        }
        if (In->Interact[1] || WentDown(Cons->rotate)) // Rotate
        {
            In->DraggedItem.size = RotateSigned(In->DraggedItem.size);
        }

    }

    // Context Menu
    if (In->ContextMenuOpened)
    {
        bb_t Bounds = {0};

        // NOTE(): Begin Context Menu
        Layout.ContextMenuMin = Bounds.min = In->ClickOffset;
        Layout.At = Bounds.min;
        ContextMenu(In, Layout, Out, User, Eq, In->ContextMenuItem, TurnSystem);
        // NOTE(): End Context Menu
        Bounds.max.x = (Bounds.min.x + Layout.ContextMenuItemWidth);
        Bounds.max.y = Layout.At.y;
        DrawBoundsOutline(Out, Bounds, Black());

        b32 ClickedOutsideBounds = (In->Interact[0] || In->Interact[1]) &&
        (IsPointInBounds(Bounds, Cursor) == false);

        if (ClickedOutsideBounds)
            In->ContextMenuOpened = false;
        if (In->ContextMenuItem == 0)
            In->CloseContextMenu = true;

        if (!In->CloseContextMenu)
        {
            In->ContextMenuT = MinF32(In->ContextMenuT + (dt * 4.0f), 1.0f);
        }
        else
        {
            In->ContextMenuT -= dt * 6.0f;
            if (In->ContextMenuT <= 0.0f)
                In->ContextMenuOpened = 0;
        }
    }
}