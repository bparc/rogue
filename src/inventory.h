fn b32 AddItemToInventory(entity_t *entity, item_t item) {
    if (entity->inventory->item_count < MAX_INVENTORY_SIZE) {
        s32 new_carried_weight = entity->inventory->carried_weight + item.params->weight;
        if (new_carried_weight <= entity->inventory->max_carry_weight) {
            entity->inventory->items[entity->inventory->item_count++] = item;
            entity->inventory->carried_weight = new_carried_weight;
            return true;
        }
    }
    return false;
}

fn b32 RemoveItemFromInventory(entity_t *entity, s32 item_id) {
    for (u8 i = 0; i < entity->inventory->item_count; i++) {
        item_t *item = &entity->inventory->items[i];

        if (item->params->id == item_id) {
            entity->inventory->carried_weight -= item->params->weight;

            for (u8 j = i; j < entity->inventory->item_count - 1; j++) {
                entity->inventory->items[j] = entity->inventory->items[j + 1];
            }

            ZeroStruct(&entity->inventory->items[entity->inventory->item_count - 1]);
            entity->inventory->item_count--;
            return true;
        }
    }
    return false;
}

// Use an item from a menu inside inventory. todo: For consumables it will consume the item;
// todo: for weapons and armor it will equip/deequip the item
fn void UseItem(entity_t *entity, s32 item_id) {
    for (s32 i = 0; i < entity->inventory->item_count; i++) {
        const item_t item = entity->inventory->items[i];
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
                    if (entity->inventory->equipped_weapon.type != item_none)
                        AddItemToInventory(entity, entity->inventory->equipped_weapon);
                    entity->inventory->equipped_weapon = item;
                    RemoveItemFromInventory(entity, item.params->id);
                    break;
                case armor:
                    if (entity->inventory->equipped_armor.type != item_none)
                        AddItemToInventory(entity, entity->inventory->equipped_armor);
                    entity->inventory->equipped_armor = item;
                    RemoveItemFromInventory(entity, item.params->id);
                    break;
                default:
                    break;
            }
            return;
        }
    }
}