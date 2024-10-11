
fn b32 AddItemToInventory(entity_t *entity, item_t *item) {
    if (entity->inventory_count < MAX_INVENTORY_SIZE) {
        u16 new_carried_weight = entity->carried_weight + item->weight;
        if (new_carried_weight <= entity->max_carry_weight) {
            entity->inventory[entity->inventory_count++] = item;
            entity->carried_weight = new_carried_weight;
            return true;
        }
    }
    return false;
}

fn b32 RemoveItemFromInventory(entity_t *entity, u32 item_id) {
    for (u8 i = 0; i < entity->inventory_count; i++) {
        item_t *item = entity->inventory[i];
        if (item != NULL && item->id == item_id) {
            entity->carried_weight -= item->weight;

            for (u8 j = i; j < entity->inventory_count - 1; j++) {
                entity->inventory[j] = entity->inventory[j + 1];
            }
            entity->inventory[entity->inventory_count - 1] = NULL;
            entity->inventory_count--;
            return true;
        }
    }
    return false;
}

fn void UseItem(entity_t *entity, u32 item_id) {
    for (u32 i = 0; i < entity->inventory_count; i++) {
        item_t *item = entity->inventory[i];
        if (item != NULL && item->id == item_id) {
            switch (item->type) {
                case ITEM_TYPE_CONSUMABLE:
                    entity->health += item->healing_amount;
                    if (entity->health > entity->max_health) {
                        entity->health = entity->max_health;
                    }
                    RemoveItemFromInventory(entity, item->id);
                    break;
                case ITEM_TYPE_WEAPON:
                    if (entity->equipped_weapon)
                        AddItemToInventory(entity, entity->equipped_weapon);
                    entity->equipped_weapon = item;
                    entity->attack_dmg = item->damage;
                    RemoveItemFromInventory(entity, item->id);
                    break;
                case ITEM_TYPE_ARMOR:
                    if (entity->equipped_armor)
                        AddItemToInventory(entity, entity->equipped_armor);
                    entity->equipped_armor = item;
                    RemoveItemFromInventory(entity, item->id);
                    break;
                case ITEM_TYPE_MISC:

                    break;
                default:
                    break;
            }
            return;
        }
    }
}