fn void RenderHitChance(command_buffer_t *out, assets_t *assets, v2 p, s32 hit_chance)
{
    v2 screen_position = ScreenToIso(p);

    char hit_chance_text[16];
    snprintf(hit_chance_text, sizeof(hit_chance_text), "%d%%", hit_chance);

    screen_position.x += 25;
    screen_position.y -= 20;

    DrawText(out, assets->Font, screen_position, hit_chance_text, White());
}

fn void DoCursor(game_world_t *Game, assets_t *assets, log_t *log,
	command_buffer_t *out, virtual_controls_t cons,
	entity_t *user, // the entity that currently uses the cursor
	b32 move_requested, s32 direction, const v2s dirs[4]) // the player wants to move)
{
	cursor_t *cursor = Game->cursor;
	slot_bar_t *bar = &Game->slot_bar;
	map_t *map = Game->map;
	entity_storage_t *storage = Game->storage;
	turn_queue_t *queue = Game->turns;

	// NOTE(): Setup
	action_t equipped = GetEquippedAction(bar, user);

	Assert(user);
	cursor->Target = 0;
	
	// NOTE(): Open the cursor.
	if ((cursor->active == false) && WentDown(cons.confirm))
	{
		switch (equipped.type)
		{
		case action_none: DebugLog("An unsupported action was selected! Can't open the cursor!"); break;
		case action_heal_self: QueryAsynchronousAction(queue, equipped.type, user, cursor->p); break;
		default:
		// NOTE(): The cursor was just activated.
		// Setup a starting state.	
			cursor->active = true;
			cursor->p = user->p;
		}
	}

	if (cursor->active)
	{
		// NOTE(): Close the cursor, if needed.
		if (WentDown(cons.cancel) || ((equipped.type == action_none || equipped.type == action_heal_self)))
			cursor->active = false;

		// NOTE(): Draw the maximum range of the cursor.
		RenderRange(out, map, user->p, equipped.params.range, Pink());

		// NOTE() : Draw the explosion radius.
		if (equipped.type == action_throw) {
			RenderRange(out, map, cursor->p, equipped.params.area_of_effect.x, Red());
		}
		// NOTE(): Draw the cursor.
		RenderIsoTile(out, map, cursor->p, A(Pink(), 0.8f), true, 0);

		// NOTE(): Move the cursor.
		v2s requestedPos = AddS(cursor->p, move_requested ? dirs[direction] : V2S(0, 0));

		b32 in_range = false;
		in_range = IsInsideCircle(requestedPos, V2S(1,1), user->p, equipped.params.range);
		if (move_requested && in_range)
				cursor->p = requestedPos;
		// NOTE(): If the cursor somehow ended up out of its range -
		// move it back to the user.
		in_range = IsInsideCircle(cursor->p, V2S(1,1), user->p, equipped.params.range);
		if ((in_range == false))
			cursor->p = user->p;

		// NOTE(): Find the closest hostile to the cursor, draw
		// tiles underneath them, then snap to them if the button went down.
		entity_t *Enemy = FindClosestHostile(storage, cursor->p);
		if (Enemy)
		{ 
			if (IsInsideCircle(Enemy->p, Enemy->size, user->p, equipped.params.range))
			{

				RenderIsoTileArea(out, map, Enemy->p, AddS(Enemy->p, Enemy->size), A(Red(), 0.8f)); //render target for all size enemies
				if (WentDown(cons.snap_cursor))
					cursor->p = Enemy->p;
			}
		}

		// NOTE(): Pick a target.
		entity_t *target = GetEntityByPosition(storage, cursor->p);
		b32 target_valid = IsHostile(target);
		if (target_valid && target)
		{
			cursor->Target = target->id;
			
			s32 hit_chance = CalculateHitChance(user, target, equipped.type);
			RenderHitChance(out, assets, MapToScreen(map, target->p), hit_chance);
		}
		// NOTE(): Perform an action on the target.
		if (WentUp(cons.confirm))
		{
			if (equipped.type == action_throw) // NOTE(): "action_throw" can also target traversable tiles.
				target_valid |= IsTraversable(map, cursor->p); 

			b32 positioned_on_user = CompareVectors(cursor->p, user->p);
			if (target_valid && (positioned_on_user == false))
			{
				QueryAsynchronousAction(queue, equipped.type, target, cursor->p);
				cursor->active = false;
			}
		}
	}
}