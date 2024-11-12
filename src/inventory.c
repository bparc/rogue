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

fn b32 Eq_RemoveItem(inventory_t *inventory, item_id_t ID)
{
    for (u8 i = 0; i < inventory->item_count; i++)
    {
        item_t *item = &inventory->items[i];
        if (item->ID == ID)
        {
            inventory->carried_weight -= item->params->weight;
            inventory->items[i] = inventory->items[--inventory->item_count];
            Eq_FreeSpace(inventory, item->index, item->params->size);
            return true;
        }
    }
    return false;
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