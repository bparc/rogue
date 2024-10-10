fn void	DoCursor(
	command_buffer_t *out,
	entity_t *User, // the entity that currently uses the cursor
	b32 exit_key, // the player wants to close the cursor
	b32 toggle_key, // the player wants to open the cursor
	b32 snap_key, // the player wants to snap onto enemy
	b32 move_requested, s32 direction, const v2s dirs[4], // the player wants to move
	turn_queue_t *queue, map_t *map, entity_storage_t *storage, log_t *log, cursor_t *cursor)
{
	Assert(User);
	if ((cursor->active == false) && toggle_key)
	{
		// NOTE(): The cursor was just activated.
		// Setup a starting state.
		cursor->active = true;
		cursor->p = User->p;
	}

	if (cursor->active)
	{
		if (move_requested)
			cursor->p = AddS(cursor->p, dirs[direction]);

		if (snap_key) {
			entity_t *nearest_enemy = FindNearestEnemy(storage, cursor->p);

			cursor->p = nearest_enemy->p;
		}

		// NOTE(): Combat!!
		entity_t *Target = GetEntityByPosition(storage, cursor->p);
		if (IsHostile(Target) == false)
			Target = 0;

		if (Target && exit_key)
		{
			PushLogLn(log, "Attacked entity %i for %i damage!", Target->id, User->base_attack_dmg);
			Target->p.x -= 4;
			cursor->active = false;
		}

		RenderIsoTile(out, map, cursor->p, SetAlpha(Pink(), 0.8f), true, 0);

		if (exit_key)
			cursor->active = false;
	}
}