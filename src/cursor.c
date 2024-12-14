fn void UpdateAndRenderCursor(game_state_t *State, cursor_t *Cursor, command_buffer_t *out,
	virtual_controls_t cons, entity_t *user, dir_input_t DirInput)
{
	// NOTE(): Setup
	action_t equipped = GetEquippedAction(&State->Bar, user);
	const action_params_t *settings = GetActionParams(equipped.type);
	const s32 Range = settings->range;
	const v2s Area  = settings->area;
	
	Cursor->Target = 0;

	RenderIsoTileArray(out, &State->Map, (IsCursorEnabled(Cursor) ? Cursor->p : user->p),
		DirInput.Dirs, ArraySize(DirInput.Dirs), Orange());

	// NOTE(): Open the Cursor.
	if (WentDown(cons.confirm) && (Cursor->active == false) && (equipped.type != action_none))
	{
		// NOTE(): Actions that only targets "self"
		// are activated directly from the menu, without opening the Cursor.
		if (IsTargetSelf(equipped.type))
		{
			CombatAction(State, user, user->p, settings);
		}
		else
		{
			// NOTE(): The Cursor was just activated.
			// Setup a starting state.	
			Cursor->active = true;
			Cursor->p = user->p;	
		}
	}

	if (Cursor->active)
	{
		// NOTE(): Close the Cursor, if needed.
		if (WentDown(cons.cancel) || ((equipped.type == action_none || equipped.type == action_heal)))
			Cursor->active = false;

		// NOTE(): Draw the maximum Range of the Cursor.
		RenderRange(out, &State->Map, user->p, Range, Pink());

		// NOTE() : Draw the explosion radius.
		if (equipped.type == action_throw) {
			RenderRange(out, &State->Map, Cursor->p, Area.x, Red()); // inner
			RenderRange(out, &State->Map, Cursor->p, Area.x * (s32)2, Red()); // outer
		}
		// NOTE(): Draw the Cursor.
		RenderIsoTile(out, &State->Map, Cursor->p, A(Pink(), 0.8f), true, 0);

		// NOTE(): Move the Cursor.
		if (DirInput.Inputed)
		{
			v2s NewPosition = IntAdd(Cursor->p, DirInput.Direction);
			if (IsInsideCircle(NewPosition, V2S(1,1), user->p, Range) &&
				IsLineOfSight(&State->Map, user->p, NewPosition))
			{
				Cursor->p = NewPosition;
			}
		}

		// NOTE(): Move the Cursor back to the user if it somehow ended up out of its range.
		if ((IsInsideCircle(Cursor->p, V2S(1,1), user->p, Range) == false))
			Cursor->p = user->p;

		// NOTE(): Find the closest hostile to the Cursor, draw
		// tiles underneath them, then snap to them if the button went down.
		entity_t *Enemy = FindClosestHostile(&State->Units, Cursor->p);
		if (Enemy && (IsInsideCircle(Enemy->p, Enemy->size, user->p, Range) &&
			IsLineOfSight(&State->Map, user->p, Enemy->p)))
		{
			RenderIsoTileArea(out, &State->Map, Enemy->p, IntAdd(Enemy->p, Enemy->size), A(Red(), 0.8f)); //render target for all size enemies
			if (WentDown(cons.SnapCursor))
				Cursor->p = Enemy->p;
		}

		// NOTE(): Pick a target.
		entity_t *target = GetEntityFromPosition(&State->Units, Cursor->p);

		if (IsHostile(target))
		{
			s32 chance = CalculateHitChance(user, target, equipped.type);
			RenderDiegeticText(&State->Camera, State->Assets->Font, target->deferred_p, V2(-20.0f, -85.0f), White(), "%i%%", chance);

			Cursor->Target = target->id;
		}

		// perform an action on the target

		if (WentUp(cons.confirm) && !CompareInts(Cursor->p, user->p))
		{
			CombatAction(State, user, Cursor->p, settings);
		}
	}
}