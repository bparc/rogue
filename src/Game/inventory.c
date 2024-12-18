fn b32 Eq_IsSpaceFree_Exclude(const inventory_t *eq, v2s offset, v2s size, item_id_t excluded)
{
    v2s min = offset;
    v2s max = IntAdd(offset, size);
    for (s32 y = min.y; y < max.y; y++)
    {
        for (s32 x = min.x; x < max.x; x++)
        {
            if ((x >= 0 && x < eq->x) &&
                (y >= 0 && y < eq->y))
            {
                item_id_t ID = (eq->layout[y][x].ID);

                b32 occupied = false;
                if (excluded == 0)
                    occupied = (ID > 0);
                else
                    occupied = (ID > 0) && (ID != excluded);

                if (occupied)
                    return false;
            }
        }
    }
    return true;
}

fn b32 Eq_IsSpaceFree(const inventory_t *eq, v2s offset, v2s size)
{
    v2s max = IntAdd(offset, size);

    if (offset.x < 0 || offset.y < 0 || max.x > eq->x || max.y > eq->y)
        return false;

    return (Eq_IsSpaceFree_Exclude(eq, offset, size, 0));
}

fn b32 Eq_IsSpaceOccupied(const inventory_t *eq, v2s offset, v2s size)
{
    b32 result = !Eq_IsSpaceFree(eq, offset, size);
    return result;
}

fn b32 Eq_FindVacantSpace(const inventory_t *Eq, v2s *Index, v2s RequiredSpace)
{
    for (s32 y = 0; y < Eq->y; y++)
    {
        for (s32 x = 0; x < Eq->x; x++)
        {
            const layout_cell_t *Cell = &Eq->layout[y][x];
            if (Cell->ID == 0)
            {
                *Index = V2S(x, y);
                return (true);
            }
        }
    }
    return (false);
}

fn void Eq_OccupySpace(inventory_t *eq, v2s offset, v2s size, item_id_t ID)
{
    v2s min = offset;
    v2s max = IntAdd(offset, size);

    if (min.x < 0 || min.y < 0 || max.x > eq->x || max.y > eq->y)
        return;

    for (s32 y = min.y; y < max.y; y++)
    {
        for (s32 x = min.x; x < max.x; x++)
        {
            if ((x >= 0 && x < eq->x) &&
                (y >= 0 && y < eq->y))
            {
                eq->layout[y][x].ID = ID;
            }
        }
    }
}

fn void Eq_FreeSpace(inventory_t *eq, v2s min, v2s size)
{
    v2s max = IntAdd(min, size);
    for (s32 y = min.y; y < max.y; y++)
    {
        for (s32 x = min.x; x < max.x; x++)
        {
            if ((x >= 0 && x < eq->x) &&
                (y >= 0 && y < eq->y))
            {
                eq->layout[y][x].ID = false;
            }
        }
    }
}

fn item_t *Eq_GetItem(inventory_t *inventory, item_id_t ID)
{
    for (s32 index = 0; index < inventory->item_count; index++)
    {
        item_t *Item = &inventory->items[index];
        if (Item->ID == ID)
            return Item;
    }
    return 0;
}

fn item_id_t Eq_AllocateID(inventory_t *inventory)
{
    item_id_t result = (1 + *inventory->global_item_count);
    *inventory->global_item_count += 1;

    return result;
}

fn item_t *Eq_PushItem(inventory_t *inventory)
{
    item_t *result = 0;
    if (inventory->item_count < ArraySize(inventory->items))
    {
        result = &inventory->items[inventory->item_count++];
        ZeroStruct(result);
    }
    return result;
}

fn void Eq_OccupyItemSpace(inventory_t *Eq, item_t Item)
{
    Eq_OccupySpace(Eq, Item.index, Item.size, Item.ID);
}

fn item_t *Eq_StoreItemUnchecked(inventory_t *Eq, item_t Item, v2s At, item_id_t ID)
{
    item_t *Result = Eq_PushItem(Eq);
    if (Result)
    {
        *Result = Item;
        Result->index = At;
        Result->ID = ID;
        Eq_OccupyItemSpace(Eq, *Result);
    }
    return Result;
}

fn item_t *Eq_AddItem(inventory_t *inventory, item_type_t type)
{
    item_t *result = NULL;

    item_t Item = ItemFromType(type);
    v2s VacantSpace = {0};
    if (Eq_FindVacantSpace(inventory, &VacantSpace, Item.size))
        result = Eq_StoreItemUnchecked(inventory, Item, VacantSpace, Eq_AllocateID(inventory));

    return (result);
}

fn b32 Eq_RemoveItem(inventory_t *inventory, item_id_t ID)
{
    for (u8 i = 0; i < inventory->item_count; i++)
    {
        item_t *item = &inventory->items[i];
        if (item->ID == ID)
        {
            Eq_FreeSpace(inventory, item->index, item->size);
            inventory->carried_weight -= item->params->weight;
            inventory->items[i] = inventory->items[--inventory->item_count];
            return true;
        }
    }
    return false;
}

fn void Eq_TransferItem(inventory_t *From, inventory_t *To, item_t Source, v2s Dest)
{
    item_t *Item = NULL;
    if (Eq_IsSpaceFree(To, Dest, Source.size))
        Item = Eq_StoreItemUnchecked(To, Source, Dest, Source.ID);
    if (Item)
        Eq_RemoveItem(From, Source.ID);   
}

fn void Eq_MoveItem(inventory_t *Eq, item_t Source, v2s Dest)
{
    item_t *Item = NULL;

    if (Eq_IsSpaceFree_Exclude(Eq, Dest, Source.size, Source.ID))
        Item = Eq_GetItem(Eq, Source.ID);    

    if (Item)
    {
        Eq_FreeSpace(Eq, Item->index, Item->size);
        Eq_OccupySpace(Eq, Dest, Source.size, Item->ID);
        Item->index = Dest;
        Item->size = Source.size;
    }
}