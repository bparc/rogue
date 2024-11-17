fn b32 Eq_IsSpaceFree_Exclude(const inventory_t *eq, v2s offset, v2s size, item_id_t excluded)
{
    v2s min = offset;
    v2s max = Add32(offset, size);
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
    v2s max = Add32(offset, size);

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
    v2s max = Add32(offset, size);

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
    v2s max = Add32(min, size);
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
    item_id_t result = inventory->next_item_ID++;
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

fn item_t *Eq_AddItem(inventory_t *inventory, item_type_t type)
{
    item_t *result = NULL;

    const item_params_t *requested_type = GetItemParams(type);
    s32 new_carried_weight = (inventory->carried_weight + requested_type->weight);
    v2s vacant_space = {0};

    if ((new_carried_weight <= inventory->max_carry_weight) &&
        (Eq_FindVacantSpace(inventory, &vacant_space, requested_type->size)))
    {
        result = Eq_PushItem(inventory);
    }

    if (result)
    {
        inventory->carried_weight = new_carried_weight;

        result->params  = requested_type;
        result->size    = requested_type->size;
        result->ID      = Eq_AllocateID(inventory);
        result->index   = vacant_space;

        Eq_OccupySpace(inventory, result->index, result->params->size, result->ID);
    }

    return (result);
}

fn item_t *Eq_AddItemAt(inventory_t *inventory, item_type_t type, v2s destPos)
{
    item_t *result = NULL;

    const item_params_t *requested_type = GetItemParams(type);
    s32 new_carried_weight = (inventory->carried_weight + requested_type->weight);

    if ((new_carried_weight <= inventory->max_carry_weight) &&
        (Eq_IsSpaceFree(inventory, destPos, requested_type->size)))
    {
        result = Eq_PushItem(inventory);
    }

    if (result)
    {
        inventory->carried_weight = new_carried_weight;

        result->params  = requested_type;
        result->size    = requested_type->size;
        result->ID      = Eq_AllocateID(inventory);
        result->index   = destPos;

        Eq_OccupySpace(inventory, result->index, result->params->size, result->ID);
    }

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

/*fn void Eq_MoveItem(inventory_t *Dest, item_t SelectedItem, v2s DestPos, inventory_t *SourceContainer)
{

    v2s maxDest = Add32(DestPos, SelectedItem.size);
    if (DestPos.x < 0 || DestPos.y < 0 || maxDest.x > Dest->x || maxDest.y > Dest->y)
        return;

    item_t *Item = NULL;

    if (SourceContainer) {
        Item = Eq_GetItem(SourceContainer, SelectedItem.ID);
    }

    if (Item)
    {
        Eq_FreeSpace(SourceContainer, Item->index, Item->size);
        Eq_OccupySpace(Dest, DestPos, SelectedItem.size, Item->ID);
        Item->index = DestPos;
        Item->size = SelectedItem.size;
    }
}*/

fn b32 Eq_MoveItem(inventory_t *Dest, inventory_t *Source, item_id_t ItemID, v2s DestPos)
{
    item_t *Item = Eq_GetItem(Source, ItemID);
    if (!Item)
        return false;

    v2s maxDest = Add32(DestPos, Item->size);
    if (DestPos.x < 0 || DestPos.y < 0 || maxDest.x > Dest->x || maxDest.y > Dest->y)
        return false;

    if (!Eq_IsSpaceFree(Dest, DestPos, Item->size)) {
        return false;
    }

    Eq_FreeSpace(Source, Item->index, Item->size);
    Source->carried_weight += Item->params->weight;

    Eq_OccupySpace(Dest, DestPos, Item->size, Item->ID);
    Item->index = DestPos;

    Dest->carried_weight += Item->params->weight;

    return true;
}

// Use an item from a menu inside inventory. todo: For consumables it will consume the item;
// todo: for weapons and armor it will equip/deequip the item
#if 0
fn void UseItem(entity_t *entity, s32 item_id)
{
    inventory_t *inventory = entity->inventory;
    for (s32 i = 0; i < inventory->item_count; i++) {
        const item_t item = inventory->items[i];
        if (item.params->id == item_id) {
            switch (item.params->category) {
                case healing:
                    entity->health += item.params->healing;
                    if (entity->health > entity->max_health) {
                        entity->health = entity->max_health;
                    }
                    Eq_RemoveItem(entity, item.params->id);
                    break;
                case rifle:
                case pistol:
                case shotgun:
                case explosive:
                    if (inventory->equipped_weapon.type != item_none)
                        Eq_AddItem(entity, inventory->equipped_weapon.type);
                    inventory->equipped_weapon = item;
                    Eq_RemoveItem(entity, item.params->id);
                    break;
                case armor:
                    if (inventory->equipped_armor.type != item_none)
                        Eq_AddItem(entity, inventory->equipped_armor.type);
                    inventory->equipped_armor = item;
                    Eq_RemoveItem(entity, item.params->id);
                    break;
                default:
                    break;
            }
            return;
        }
    }
}
#endif