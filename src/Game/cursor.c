fn void RenderHitChance(command_buffer_t *out, assets_t *assets, v2 p, s32 hit_chance)
{
    v2 screen_position = ScreenToIso(p);

    char hit_chance_text[16];
    snprintf(hit_chance_text, sizeof(hit_chance_text), "%d%%", hit_chance);

    screen_position.x += 25;
    screen_position.y -= 20;

    DrawText(out, assets->Font, screen_position, hit_chance_text, White());
}

fn inline s32 Cursor_DoAction(cursor_t *cursor, map_t *Map, entity_t *user, entity_t *target, game_state_t *queue, const action_params_t *settings)
{
	entity_id_t id = target ? target->id : 0;
	v2s TargetedCell = target ? target->p : cursor->p;

	b32 TargetValid = true;
    if (IsTargetField(settings->type))
    	TargetValid = (TargetValid && IsTraversable(Map, cursor->p));
    if (IsTargetHostile(settings->type))
    	TargetValid = (TargetValid && IsHostile(target));

	b32 Query = false;
    if (IsLineOfSight(Map, user->p, TargetedCell) && TargetValid)
       	Query = ConsumeActionPoints(queue, settings->cost);

    if (Query)
		QueryAsynchronousAction(queue, settings->type, id, TargetedCell);
	return Query;
}

fn void DoCursor(
	game_state_t *State,
	camera_t *Camera,
	cursor_t *cursor,
	assets_t *Assets,
	slot_bar_t *bar,
	command_buffer_t *out,
	virtual_controls_t cons,
	entity_t *user,
	dir_input_t DirInput)
{
	map_t *Map = &State->Map;
	entity_storage_t *storage = &State->Units;
	game_state_t *queue = State;

	// NOTE(): Setup
	action_t equipped = GetEquippedAction(bar, user);
	const action_params_t *settings = GetParameters(equipped.type);
	const s32 range = settings->range;
	const v2s area  = settings->area;
	cursor->Target = 0;
	
	// NOTE(): Render
	{
		v2s CursorPos = IsCursorEnabled(cursor) ? cursor->p : user->p;
		for (s32 Index = 0; Index < ArraySize(DirInput.Dirs); Index++)	
		RenderIsoTile(out, Map, IntAdd(CursorPos, DirInput.Dirs[Index]), Orange(), true, 0);
	}

	// NOTE(): Open the cursor.
	if (WentDown(cons.confirm) && (cursor->active == false) && (equipped.type != action_none))
	{
		// NOTE(): Actions that only targets "self"
		// are activated directly from the menu, without opening the cursor.
		if (IsTargetSelf(equipped.type))
		{
			Cursor_DoAction(cursor, Map, user, user, queue, settings);
		}
		else
		{
			// NOTE(): The cursor was just activated.
			// Setup a starting state.	
			cursor->active = true;
			cursor->p = user->p;	
		}
	}

	if (cursor->active)
	{
		// NOTE(): Close the cursor, if needed.
		if (WentDown(cons.cancel) || ((equipped.type == action_none || equipped.type == action_heal)))
			cursor->active = false;

		// NOTE(): Draw the maximum range of the cursor.
		RenderRange(out, Map, user->p, range, Pink());

		// NOTE() : Draw the explosion radius.
		if (equipped.type == action_throw) {
			RenderRange(out, Map, cursor->p, area.x, Red()); // inner
			RenderRange(out, Map, cursor->p, area.x * (s32)2, Red()); // outer
		}
		// NOTE(): Draw the cursor.
		RenderIsoTile(out, Map, cursor->p, A(Pink(), 0.8f), true, 0);

		// NOTE(): Move the cursor.
		if (DirInput.Inputed)
		{
			v2s NewPosition = IntAdd(cursor->p, DirInput.Direction);
			if (IsInsideCircle(NewPosition, V2S(1,1), user->p, range) &&
				IsLineOfSight(Map, user->p, NewPosition))
			{
				cursor->p = NewPosition;
			}
		}

		// NOTE(): Move the cursor back to the user if it somehow ended up out of its range.
		if ((IsInsideCircle(cursor->p, V2S(1,1), user->p, range) == false))
			cursor->p = user->p;

		// NOTE(): Find the closest hostile to the cursor, draw
		// tiles underneath them, then snap to them if the button went down.
		entity_t *Enemy = FindClosestHostile(storage, cursor->p);
		if (Enemy && (IsInsideCircle(Enemy->p, Enemy->size, user->p, range) &&
			IsLineOfSight(Map, user->p, Enemy->p)))
		{
			RenderIsoTileArea(out, Map, Enemy->p, IntAdd(Enemy->p, Enemy->size), A(Red(), 0.8f)); //render target for all size enemies
			if (WentDown(cons.SnapCursor))
				cursor->p = Enemy->p;
		}

		// NOTE(): Pick a target.
		entity_t *target = GetEntityByPosition(storage, cursor->p);

		if (IsHostile(target))
		{
			s32 chance = CalculateHitChance(user, target, equipped.type);
			RenderDiegeticText(Camera, Assets->Font, target->deferred_p, V2(-20.0f, -85.0f), White(), "%i%%", chance);

			cursor->Target = target->id;
		}
		// NOTE(): Perform an action on the target.
		b32 not_positioned_on_user = !CompareInts(cursor->p, user->p);
		if (WentUp(cons.confirm) && not_positioned_on_user)
			Cursor_DoAction(cursor, Map, user, target, queue, settings);
	}
}