fn void DrawCursorArea(command_buffer_t *out, map_t *map,v2s center, int radius) {
    for (s32 y = center.y - radius; y <= center.y + radius; ++y)
    {
        for (s32 x = center.x - radius; x <= center.x + radius; ++x)
        {

            v2s point = V2S(x,y);

            if (IsInsideCircle(point, V2S(1,1), center, radius))
            {
                RenderIsoTile(out, map, point, SetAlpha(Pink(), 0.5f), true, 0);
            }
        }
    }
}

fn void ActivateSlotAction(entity_t *user, entity_t *target, action_type_t action)
{
    switch(action) {
        case action_ranged_attack:
        	{
            	InflictDamage(target, user->attack_dmg);
            	break;
        	}
        case action_melee_attack:
        	{
        		InflictDamage(target, user->attack_dmg);
        		target->p.x -= 4; // NOTE(): Push-back
        		break;
        	}
        case action_none:
        default:
            break;
    }
}

fn void	DoCursor(
	command_buffer_t *out,
	entity_t *User, // the entity that currently uses the cursor
	virtual_controls_t cons,
	b32 move_requested, s32 direction, const v2s dirs[4], // the player wants to move
	turn_queue_t *queue, map_t *map, entity_storage_t *storage, log_t *log, cursor_t *cursor,
	slot_bar_t bar)
{
	Assert(User);
	if ((cursor->active == false) && WentDown(cons.confirm))
	{
		// NOTE(): The cursor was just activated.
		// Setup a starting state.
		cursor->active = true;
		cursor->p = User->p;
	}

	if (cursor->active)
	{
		// NOTE(): Close the cursor, if needed.
		if (WentDown(cons.cancel))
			cursor->active = false;

		DebugAssert((bar.selected_slot >= 0) &&
		(bar.selected_slot < ArraySize(bar.slots))); // NOTE(): Validate, just in case.
		action_type_t equipped = bar.slots[bar.selected_slot - 1].action; // NOTE(): Slot IDs start from 1?
		if (equipped == action_none)
		{
			DebugLog("An unsupported action (%s) was selected! Can't open the cursor!", action_type_t_names[equipped]);
			cursor->active = false;
			return;
		}

		// NOTE(): Load up a "cursor specification" from the
		// currently  equipped ability.
		// NOTE(): We can have like different cursor patterns here or whatnot.
		s32 Range = equipped == action_ranged_attack ? 4 :
					equipped == action_melee_attack  ? 2
					: 0;

		// NOTE(): Draw the maximum range of the cursor.
		DrawCursorArea(out, map, User->p, Range);
		// NOTE(): Draw the cursor.
		RenderIsoTile(out, map, cursor->p, SetAlpha(Pink(), 0.8f), true, 0);

		// NOTE(): Move the cursor.
		v2s requestedPos = AddS(cursor->p, move_requested ? dirs[direction] : V2S(0, 0));

		b32 in_range = false;
		in_range = IsInsideCircle(requestedPos, V2S(1,1), User->p, Range);
		if (move_requested && in_range)
				cursor->p = requestedPos;
		// NOTE(): If the cursor somehow ended up out of its range -
		// move it back to the user.
		in_range = IsInsideCircle(cursor->p, V2S(1,1), User->p, Range);
		if ((in_range == false))
			cursor->p = User->p;

		// NOTE(): Find the closest hostile to the cursor, draw
		// tiles underneath them, then snap to them if the button went down.
		entity_t *Enemy = FindClosestHostile(storage, cursor->p);
		if (Enemy)
		{ 
			if (IsInsideCircle(Enemy->p, Enemy->size, User->p, Range))
			{
				RenderIsoTileArea(out, map, Enemy->p, AddS(Enemy->p, Enemy->size), SetAlpha(Red(), 0.8f)); //render target for all size enemies
				if (WentDown(cons.snap_cursor))
					cursor->p = Enemy->p;
			}
		}

		// NOTE(): Pick the target under the cursor and perform the "slot action" on it.
		entity_t *Target = GetEntityByPosition(storage, cursor->p);
		if (IsHostile(Target) && WentDown(cons.confirm))
		{
			ActivateSlotAction(User, Target, equipped);
			cursor->active = false;
		}
	}
}