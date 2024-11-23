fn void BeginInterface(interface_t *In, game_state_t *GameState, const client_input_t *Input, const virtual_controls_t *Cons, command_buffer_t *Out, f32 dt)
{
    In->Cons = Cons;
    In->Input = Input;
    In->Out = Out;
    In->Font = GameState->assets->Font;
    In->TurnSystem = GameState->turns;
    In->DeltaTime = dt;

    In->Cursor = GetCursorOffset(Input);
    In->Interact[0] = (!In->Buttons[0] && Input->mouse_buttons[0]);
    In->Interact[1] = (!In->Buttons[1] && Input->mouse_buttons[1]);

    In->Buttons[0] = Input->mouse_buttons[0];
    In->Buttons[1] = Input->mouse_buttons[1];
}

fn void EndInterface(interface_t *In)
{

}

fn void HUD(command_buffer_t *out, game_state_t *state, const client_input_t *input, const virtual_controls_t *Cons, f32 dt)
{
    BeginInterface(state->interface, state, input, Cons, out, dt);

    entity_t *ActiveEntity = GetActive(state->turns);
    if (state->turns->EncounterModeEnabled)
    {
        TurnQueue(out, state, state->turns, state->assets, state->cursor);
    }

    interface_t *In = state->interface;
    if (IsPlayer(ActiveEntity))
    {   
        MiniMap(Debug.out_top, state->layout, ActiveEntity->p, state->assets);
        
        if (In->InventoryOpened || In->OpenedContainer)
        {
            Inventory(In, V2(100.0f, 25.0f), ActiveEntity->inventory, ActiveEntity, NULL, true);

            container_t *OpenedContainer = In->OpenedContainer;
            if (In->OpenedContainer)
            {
                Inventory(In, V2(800.0f, 25.0f), &OpenedContainer->inventory, ActiveEntity, OpenedContainer, false);
            }
        }

        ActionMenu(ActiveEntity, state, out, state->assets, input, state->turns);
    }
    
    EndInterface(state->interface);
}