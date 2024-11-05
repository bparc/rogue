fn b32 FindVacantSpace(const inventory_t *Eq, v2s *Index, v2s RequiredSpace)
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

fn item_t *AddItemToInventory(entity_t *entity, item_type_t type)
{
    item_t item = ItemFromType(type);
    const item_params_t *Params = item.params;

    inventory_t *inventory = entity->inventory;

    v2s VacantSpace = {0};
    if ((inventory->item_count < ArraySize(inventory->items)) &&
        FindVacantSpace(inventory, &VacantSpace, Params->EqSize))
    {
        s32 new_carried_weight = inventory->carried_weight + item.params->weight;
        if (new_carried_weight <= inventory->max_carry_weight)
        {
            item_t *Item = &inventory->items[inventory->item_count++];
            *Item = item;
            Item->x = VacantSpace.x;
            Item->y = VacantSpace.y;
            inventory->carried_weight = new_carried_weight;

            // NOTE(): Occupy
            inventory->layout[VacantSpace.y][VacantSpace.x].ID = true;

            return Item;
        }
    }
    return 0;
}

fn b32 RemoveItemFromInventory(entity_t *entity, s32 item_id)
{
    inventory_t *inventory = entity->inventory;
    for (u8 i = 0; i < inventory->item_count; i++) {
        item_t *item = &inventory->items[i];

        if (item->params->id == item_id) {
            inventory->carried_weight -= item->params->weight;

            for (u8 j = i; j < inventory->item_count - 1; j++) {
                inventory->items[j] = inventory->items[j + 1];
            }

            ZeroStruct(&inventory->items[inventory->item_count - 1]);
            inventory->item_count--;
            return true;
        }
    }
    return false;
}

// Use an item from a menu inside inventory. todo: For consumables it will consume the item;
// todo: for weapons and armor it will equip/deequip the item
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
                    RemoveItemFromInventory(entity, item.params->id);
                    break;
                case rifle:
                case pistol:
                case shotgun:
                case explosive:
                    if (inventory->equipped_weapon.type != item_none)
                        AddItemToInventory(entity, inventory->equipped_weapon.type);
                    inventory->equipped_weapon = item;
                    RemoveItemFromInventory(entity, item.params->id);
                    break;
                case armor:
                    if (inventory->equipped_armor.type != item_none)
                        AddItemToInventory(entity, inventory->equipped_armor.type);
                    inventory->equipped_armor = item;
                    RemoveItemFromInventory(entity, item.params->id);
                    break;
                default:
                    break;
            }
            return;
        }
    }
}