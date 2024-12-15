fn entity_t *CreatePlayer(game_state_t *State, v2s p)
{
    entity_t *result = 0;
    if (State->Players[0] == 0)
    {
        u16 player_health = 62;
        u16 player_max_health = 62;
        u16 attack_dmg = 8; // What does this do now?
        s32 player_accuracy = 75; // Applying this value for both melee and ranged accuracy
        s32 player_evasion = 20;
        result = CreateEntity(State, p, V2S(1, 1), entity_flags_controllable,
            player_health, attack_dmg, &State->Map, player_max_health, player_accuracy, player_evasion,
            MAX_PLAYER_ACTION_POINTS, MAX_PLAYER_MOVEMENT_POINTS, 1);
        
	    result->inventory = CreateInventory(State); 
        
        inventory_t *Inventory = result->inventory;
        Eq_AddItem(Inventory, item_green_herb);
        Eq_AddItem(Inventory, item_assault_rifle);
        Eq_AddItem(Inventory, item_freezing_spell);

        for (int32_t Index = 0; Index < Inventory->item_count; Index++)
        {
            AssignItem(&State->Bar, Inventory->items[Index].ID, (s8)Index);
        }

        DebugWarning("");

        State->Players[0] = result->id;
    }
    else
    {
        DebugLog("player already created!");
    }
    return result;
}

fn inline void UpdatePlayer(game_state_t *State, entity_t *Entity, const client_input_t *input, const virtual_controls_t *cons, dir_input_t DirInput, b32 BlockInputs)
{
	// check adjacent containers

	container_t *Container = GetAdjacentContainer(State, Entity->p);
	if (Container)
	{
		OpenContainer(&State->GUI, Container);
	}

	if(!Container || (State->GUI.OpenedContainer != Container))
	{
		CloseContainer(&State->GUI);
	}

	// check adjacent doors

	v2s DoorIndex = {0};
	if (GetAdjacentDoor(State, Entity->p, &DoorIndex))
	{
		RenderDiegeticText(&State->Camera, State->Assets->Font, Entity->deferred_p, V2(-10.0f, -50.0f), White(), "Press R to open.");
		if (IsKeyPressed(input, 'R'))
			SetTileValue(&State->Map, DoorIndex, tile_floor);
	}

	// move

	b32 AllowedToMove = IsActionQueueCompleted(State)  && // Can't move when skill animations are playing!
		((BlockInputs == false));

	if (State->EncounterModeEnabled)
	{
		if (!CheckRange(&State->EffectiveRange, IntAdd(Entity->p, DirInput.Direction)))
		{
			AllowedToMove = false;
		}
	}	
	
	if (DirInput.Inputed && AllowedToMove)
	{
		MakeMove(State, Entity, DirInput.Direction);
	}

	// end

	b32 Skip = WentDown(cons->EndTurn);
	if ((Skip || (State->ActionPoints == 0)) && State->EncounterModeEnabled)
	{
		if (Skip)
		{
			Brace(State, Entity);
			CloseInventory(&State->GUI);
		}
		EndTurn(State);
	}
}