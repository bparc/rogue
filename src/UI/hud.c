fn void HUD(command_buffer_t *out,game_state_t *state, turn_queue_t *queue, entity_storage_t *storage,
    assets_t *assets, const client_input_t *input, const virtual_controls_t *Cons, f32 dt)
{
    BeginInterface(state->interface, input);

    entity_t *ActiveEntity = GetActiveUnit(queue);

    TurnQueue(out, state, queue, assets, state->cursor);

    interface_t *In = state->interface;
    if (IsPlayer(ActiveEntity))
    {   
        MiniMap(Debug.out_top, state->layout, ActiveEntity->p, assets);
        
        if (In->InventoryOpened)
        {
            Inventory(V2(100.0f, 25.0f), out, ActiveEntity->inventory, input, assets->Font, state->interface, ActiveEntity, Cons, dt, true, NULL);
            if (In->OpenedContainer)
            {
                Inventory(V2(800.0f, 25.0f), out, &In->OpenedContainer->inventory, input, assets->Font, state->interface, ActiveEntity, Cons, dt, false, In->OpenedContainer);
            }
        }

        ActionMenu(ActiveEntity, state, out, assets, input, queue);
    }
    
    EndInterface(state->interface);
}