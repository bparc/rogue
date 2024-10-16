void DrawCursorArea(command_buffer_t *out, map_t *map,v2s center, int radius) {
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
		//draw cursorable area
		DrawCursorArea(out, map, User->p, 5);
		if (move_requested){
			v2s requestedPos = AddS(cursor->p, dirs[direction]);

			if(IsInsideCircle(requestedPos, V2S(1,1), User->p, 5)){ //default cursor is 1x1 size
				cursor->p = AddS(cursor->p, dirs[direction]);
			}
		}

		// NOTE(): Targeting/Snaping to an enemy
		{
			entity_t *nearest_enemy = FindNearestEnemy(storage, cursor->p);
			if(nearest_enemy) { //if found
				v2s pos = nearest_enemy->p;
				v2s size = nearest_enemy->size;
				//render target for all size enemies
				for (int y = pos.y; y < size.y+pos.y; ++y) {
					for (int x = pos.y; x < size.x + pos.x; ++x) {
						RenderIsoTile(out, map, V2S(x,y), SetAlpha(Red(), 0.8f), true, 0);
					}
				}

				
				if (WentDown(cons.x)) {
					if (nearest_enemy && IsInsideCircle(nearest_enemy->p, nearest_enemy->size, User->p, 5))
					{
						cursor->p = nearest_enemy->p;
						
					}
				}
			}
		}
		// NOTE(): Combat!!
		entity_t *Target = GetEntityByPosition(storage, cursor->p);
		if (IsHostile(Target) == false)
			Target = 0;

		if (Target && WentDown(cons.confirm))
		{
			InflictDamage(Target, User->attack_dmg);
			cursor->active = false;

			ConsumeActionPoints(queue, 1);
		}

		RenderIsoTile(out, map, cursor->p, SetAlpha(Pink(), 0.8f), true, 0);

		if (WentDown(cons.cancel))
			cursor->active = false;
	}
}