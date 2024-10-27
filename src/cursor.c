fn void RenderHitChance(command_buffer_t *out, assets_t *assets, v2 p, s32 hit_chance)
{
    v2 screen_position = ScreenToIso(p);

    char hit_chance_text[16];
    snprintf(hit_chance_text, sizeof(hit_chance_text), "%d%%", hit_chance);

    screen_position.x += 25;
    screen_position.y -= 20;

    DrawText(out, assets->Font, screen_position, hit_chance_text, White());
}

fn void DrawDiegeticText(game_world_t *game, v2 p, v2 offset, v4 color, const char *format, ...)
{
	char string[256] = "";
	va_list args = {0};
	va_start(args, format);
	vsnprintf(string, sizeof(string), format, args);
	va_end(args);

	command_buffer_t *out = Debug.out_top;
	v2 screen_p = CameraToScreen(game, p);
	screen_p = Add(screen_p, offset);
	DrawText(out, game->assets->Font, screen_p, string, color);
}

fn inline s32 DoAction(cursor_t *cursor, map_t *map, entity_t *user, entity_t *target, turn_queue_t *queue, const action_params_t *settings)
{
	entity_id_t id = target ? target->id : 0;
	v2s p = target ? target->p : cursor->p;

	b32 Query = false;
	b32 LOSTest = IsLineOfSight(map, user->p, p);
    if (LOSTest)
       	Query = ConsumeActionPoints(queue, queue->god_mode_enabled ? 0 : settings->cost);
    if (Query)
		QueryAsynchronousAction(queue, settings->type, id, p);
	return Query;
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
	const action_params_t *settings = GetParameters(equipped.type);
	const s32 range = settings->range;
	const v2s area  = settings->area;
	cursor->Target = 0;
	
	// NOTE(): Open the cursor.
	if (WentDown(cons.confirm) && (cursor->active == false) && (equipped.type != action_none))
	{
		// NOTE(): Actions that only targets "self"
		// are activated directly from the menu, without opening the cursor.
		if (IsTargetSelf(equipped.type))
		{
			DoAction(cursor, map, user, user, queue, settings);
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
		RenderRange(out, map, user->p, range, Pink());

		// NOTE() : Draw the explosion radius.
		if (equipped.type == action_throw) {
			RenderRange(out, map, cursor->p, area.x, Red()); // inner
			RenderRange(out, map, cursor->p, area.x * (s32)2, Red()); // outer
		}
		// NOTE(): Draw the cursor.
		RenderIsoTile(out, map, cursor->p, A(Pink(), 0.8f), true, 0);

		// NOTE(): Move the cursor.
		v2s requested_p = Add32(cursor->p, move_requested ? dirs[direction] : V2S(0, 0));
		if (move_requested && IsInsideCircle(requested_p, V2S(1,1), user->p, range) && IsLineOfSight(map, user->p, requested_p))
				cursor->p = requested_p;

		// NOTE(): If the cursor somehow ended up out of its range -
		// move it back to the user.
		if ((IsInsideCircle(cursor->p, V2S(1,1), user->p, range) == false))
			cursor->p = user->p;

		// NOTE(): Find the closest hostile to the cursor, draw
		// tiles underneath them, then snap to them if the button went down.
		entity_t *Enemy = FindClosestHostile(storage, cursor->p);
		if (Enemy && (IsInsideCircle(Enemy->p, Enemy->size, user->p, range) &&
			IsLineOfSight(map, user->p, Enemy->p)))
		{
			RenderIsoTileArea(out, map, Enemy->p, Add32(Enemy->p, Enemy->size), A(Red(), 0.8f)); //render target for all size enemies
			if (WentDown(cons.snap_cursor))
				cursor->p = Enemy->p;
		}

		// NOTE(): Pick a target.
		entity_t *target = GetEntityByPosition(storage, cursor->p);
		if (IsHostile(target))
		{
			s32 chance = CalculateHitChance(user, target, equipped.type);
			DrawDiegeticText(Game, target->deferred_p, V2(-20.0f, -85.0f), White(), "%i%%", chance);

			cursor->Target = target->id;
		}
		// NOTE(): Perform an action on the target.
		if (WentUp(cons.confirm))
		{
			b32 target_valid = IsHostile(target);
			if (IsTargetAny(equipped.type))
				target_valid |= IsTraversable(map, cursor->p); 

			b32 positioned_on_user = CompareVectors(cursor->p, user->p);
			if (target_valid && (positioned_on_user == false))
				DoAction(cursor, map, user, target, queue, settings);
		}
	}
}