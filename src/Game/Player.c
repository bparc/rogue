fn inline void SetupPlayer(game_state_t *World, entity_t *Player)
{
	inventory_t *Inventory = Player->inventory;
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_assault_rifle);
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_assault_rifle);
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_green_herb);
    Eq_AddItem(Inventory, item_green_herb);
}

fn inline void Player(entity_t *Entity, game_state_t *state, const client_input_t *input, command_buffer_t *out, const virtual_controls_t *cons)
{
	turn_system_t *queue = state->turns;
	// NOTE(): Controls
	dir_input_t DirInput = GetDirectionalInput(input);
	b32 CursorEnabled = IsCursorEnabled(state->cursor);
	DoCursor(state, out, *cons, Entity, DirInput);
			
	if (WentDown(cons->Inventory))
		ToggleInventory(state->interface);

	// NOTE(): Check Containers
	container_t *AdjacentContainer = NULL;
	for (s32 DirIndex = 0;
		DirIndex < 4;
		DirIndex++)
	{
		v2s Dir = cardinal_directions[DirIndex];
		v2s Adjacent = Add32(Entity->p, Dir);
		container_t *Container = GetContainer(state, Adjacent);
		if (Container)
		{
			AdjacentContainer = Container;
			DrawDiegeticText(state, Entity->deferred_p, V2(-10.0f, -50.0f), White(), "Press R to open.");
			if (IsKeyPressed(input, 'R'))
			{
				OpenContainer(state->interface, Container);
			}
			break;
		}
	}

	b32 ContainerOutOfRange = state->interface->OpenedContainer && !AdjacentContainer;
	if (ContainerOutOfRange)
		CloseContainer(state->interface);

	// NOTE(): Move
	b32 AllowedToMove = IsActionQueueCompleted(queue) /* Can't move when skill animations are playing! */ &&
		(CursorEnabled == false) && (queue->movement_points > 0);
	if (DirInput.Inputed && AllowedToMove)
	{
		b32 Moved = Move(queue, Entity, DirInput.Direction);
		if (Moved && (queue->god_mode_enabled == false))
		{
			ConsumeMovementPoints(queue, 1);
			ApplyTileEffects(state->map, Entity);
		}
	}

	// NOTE(): Finish
	b32 CantDoAnyAction = (queue->movement_points <= 0 && queue->action_points == 0);
	b32 TurnForcefullySkipped = WentDown(cons->EndTurn);
	b32 EndTurn = TurnForcefullySkipped || CantDoAnyAction;
	if (EndTurn)
	{
		if (TurnForcefullySkipped)
		{
			Brace(queue, Entity);
			CloseInventory(state->interface);
		}
		AcceptTurn(queue, Entity);
	}
}