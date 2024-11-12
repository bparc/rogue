#include "hud_inventory.c"
#include "hud_bar.c"
#include "hud_queue.c"

fn void HUD(command_buffer_t *out,game_world_t *state, turn_queue_t *queue, entity_storage_t *storage, assets_t *assets, const client_input_t *input)
{
    BeginInterface(state->interface, input);

    entity_t *ActiveEntity = GetActiveUnit(queue);

    TurnQueue(out, state, queue, assets, state->cursor);

    if (IsPlayer(ActiveEntity))
    {
        if (state->interface->inventory_visible)
        {
            Inventory(out, ActiveEntity->inventory, input, assets->Font, state->interface, ActiveEntity);
        }
        
        ActionMenu(ActiveEntity, state, out, assets, input, queue);
    }

    EndInterface(state->interface);
}