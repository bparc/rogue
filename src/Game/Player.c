fn inline void SetupPlayer(game_state_t *World, entity_t *UpdatePlayer)
{
	inventory_t *Inventory = UpdatePlayer->inventory;
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_assault_rifle);
    #if 0
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_assault_rifle);
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_assault_rifle);
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_green_herb);
    #endif
}

fn container_t *GetAdjacentContainer(game_state_t *State, v2s Cell)
{
	container_t *Result = 0;
	for (s32 DirIndex = 0; DirIndex < 4;DirIndex++)
	{
		v2s AdjacentCell = IntAdd(Cell, cardinal_directions[DirIndex]);
		container_t *Container = GetContainer(State, AdjacentCell);
		if (Container)
		{
			Result = Container;		
			break;
		}
	}
	return Result;
}

fn b32 GetAdjacentDoor(game_state_t *State, v2s Cell, v2s *DoorCell)
{
	b32 Result = 0;
	for (s32 DirIndex = 0; (DirIndex < 4) && !Result; DirIndex++)
	{
		*DoorCell = IntAdd(Cell, cardinal_directions[DirIndex]);
		Result = IsDoor(&State->Map, *DoorCell);
	}
	return Result;
}

fn inline void UpdatePlayer(game_state_t *State, entity_t *Entity, const client_input_t *input, const virtual_controls_t *cons, dir_input_t DirInput, b32 BlockInputs)
{
	// inventory		

	if (WentDown(cons->Inventory))
		ToggleInventory(&State->GUI);

	// check containers

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
		EndTurn(State, Entity);
	}
}