fn inline void SetupPlayer(game_state_t *World, entity_t *Player)
{
	inventory_t *Inventory = Player->inventory;
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
		Result = IsDoor(State->Map, *DoorCell);
	}
	return Result;
}

fn inline void Player(entity_t *Entity, game_state_t *state, const client_input_t *input, command_buffer_t *out, const virtual_controls_t *cons, dir_input_t DirInput, b32 BlockInputs)
{
	turn_system_t *queue = state->System;
	// NOTE(): Controls		
	if (WentDown(cons->Inventory))
		ToggleInventory(state->interface);

	// NOTE(): Check Containers
	container_t *Container = GetAdjacentContainer(state, Entity->p);
	if (Container)
	{
		OpenContainer(state->interface, Container);
	}

	if(!Container || (state->interface->OpenedContainer != Container))
	{
		CloseContainer(state->interface);
	}

	// NOTE(): Check Doors
	v2s DoorIndex = {0};
	if (GetAdjacentDoor(state, Entity->p, &DoorIndex))
	{
		RenderDiegeticText(state->Camera, state->assets->Font, Entity->deferred_p, V2(-10.0f, -50.0f), White(), "Press R to open.");
		if (IsKeyPressed(input, 'R'))
			SetTileValue(state->Map, DoorIndex, tile_floor);
	}

	// NOTE(): Move

	b32 AllowedToMove = IsActionQueueCompleted(queue)  && // Can't move when skill animations are playing!
		((BlockInputs == false));

	if (queue->EncounterModeEnabled)
	{
		RenderRangeMap(out, queue->Map, &queue->EffectiveRange);

		if (!CheckRange(&queue->EffectiveRange, IntAdd(Entity->p, DirInput.Direction)))
		{
			AllowedToMove = false;
		}
	}	
	
	if (DirInput.Inputed && AllowedToMove)
	{
		b32 Moved = MakeMove(queue, Entity, DirInput.Direction);
		if (Moved)
		{
			room_t *Room = RoomFromPosition(state->layout, Entity->p);
			if (Room)
				Room->Visited = true;
		}

		//if (Moved && GodModeDisabled(queue) && queue->EncounterModeEnabled)
		//{
			//ConsumeMovementPoints(queue, 1);
		//}
	}

	// NOTE(): Finish
	b32 CantDoAnyAction = (queue->action_points == 0);
	b32 TurnForcefullySkipped = WentDown(cons->EndTurn);
	b32 EndTurn = TurnForcefullySkipped || CantDoAnyAction;
	if (EndTurn && queue->EncounterModeEnabled)
	{
		if (TurnForcefullySkipped)
		{
			Brace(queue, Entity);
			CloseInventory(state->interface);
		}
		AcceptTurn(queue, Entity);
	}
}