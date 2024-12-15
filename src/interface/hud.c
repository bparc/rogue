fn void BeginInterface(interface_t *In, game_state_t *State, const client_input_t *Input, const virtual_controls_t *Cons, command_buffer_t *Out, f32 dt)
{
    In->Cons = Cons;
    In->Input = Input;
    In->Out = Out;
    In->Font = State->Assets->Font;
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

fn void HUD(command_buffer_t *out, game_state_t *State, const client_input_t *input, const virtual_controls_t *Cons, f32 dt)
{
    BeginInterface(&State->GUI, State, input, Cons, out, dt);

    entity_t *ActiveEntity = GetActive(State);
    if (State->EncounterModeEnabled)
    {
        TurnQueue(out, State, State->Assets, &State->Cursor);
    }

    interface_t *In = &State->GUI;
    if (IsPlayer(ActiveEntity))
    {   
        MiniMap(Debug.out_top, &State->MapLayout, ActiveEntity->p, State->Assets);
        
        if (In->InventoryOpened || In->OpenedContainer)
        {
            Inventory(State, In, V2(100.0f, 25.0f), ActiveEntity->inventory, ActiveEntity, NULL, true);

            container_t *OpenedContainer = In->OpenedContainer;
            if (In->OpenedContainer)
            {
                Inventory(State, In, V2(800.0f, 25.0f), &OpenedContainer->inventory, ActiveEntity, OpenedContainer, false);
            }
        }

        ActionMenu(ActiveEntity, State, out, State->Assets, input, State, &State->GUI, ActiveEntity);

        if (WentDown(Cons->Inventory))
            ToggleInventory(&State->GUI);
    }
    
    EndInterface(&State->GUI);
}