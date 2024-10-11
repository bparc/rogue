fn void	DoCursor(
	command_buffer_t *out,
	entity_t *User, // the entity that currently uses the cursor
	virtual_controls_t cons,
	b32 move_requested, s32 direction, const v2s dirs[4], // the player wants to move
	turn_queue_t *queue, map_t *map, entity_storage_t *storage, log_t *log, cursor_t *cursor)
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
		if (move_requested)
			cursor->p = AddS(cursor->p, dirs[direction]);

		// NOTE(): Targeting/Snaping to an enemy
		{
			entity_t *nearest_enemy = FindNearestEnemy(storage, cursor->p);
			RenderIsoTile(out, map, nearest_enemy->p, SetAlpha(Red(), 0.8f), true, 0);
			if (WentDown(cons.x)) {
				if (nearest_enemy)
				{
					cursor->p = nearest_enemy->p;
					
				}
			}
		}
		// NOTE(): Combat!!
		entity_t *Target = GetEntityByPosition(storage, cursor->p);
		if (IsHostile(Target) == false)
			Target = 0;

		if (Target && WentDown(cons.confirm))
		{
			LogLn(log, "Attacked entity %i for %i damage!", (s32)Target->id, (s32)User->attack_dmg);
			// Target->p.x -= 4;

			// NOTE(): The health is stored in a signed integer -
			// we need to compute in an intermediate register
			// so that we won't underflow.
			s32 health = MaxS32(0, Target->health - User->attack_dmg);
			Target->health = (s16)health;
			cursor->active = false;
		}

		RenderIsoTile(out, map, cursor->p, SetAlpha(Pink(), 0.8f), true, 0);

		if (WentDown(cons.cancel))
			cursor->active = false;
	}
}