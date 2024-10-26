fn void EstablishTurnOrder(game_world_t *game, turn_queue_t *queue)
{
	DefaultTurnOrder(queue);
}

fn s32 BeginTurn(game_world_t *game, entity_t *entity)
{
	ProcessStatusEffects(entity);

	s32 movement_point_count = 10;
	if (IsHostile(entity))
	{
		//movement_point_count = 2 + (rand() % 2);

		// NOTE(): Request a path to the player.
		turn_queue_t *queue = game->turns;
		entity_t *DebugPlayer = DEBUGGetPlayer(queue->storage);
		Assert(DebugPlayer);

		path_t *path = &queue->path;
		if (!FindPath(game->map, entity->p, DebugPlayer->p, path, game->memory))
			DebugLog("Couldn't find a path!");
		movement_point_count = 6;
		queue->max_action_points = movement_point_count;

		path->length = Min32(path->length, movement_point_count);

		// NOTE(): Truncate path to the closest unoccupied point to the
		// destination.
		s32 index = path->length - 1;
		while (index >= 0)
		{
			if (IsWorldPointEmpty(game, path->tiles[index].p) == false)
			{
				index--;
				continue;
			}
			break;
		}
		movement_point_count = path->length = (index + 1);
		
		entity->DEBUG_step_count = 0;
	}

	return movement_point_count;
}

fn s32 Decide(game_world_t *game, entity_t *entity)
{
	#if 1
	path_t *path = &game->turns->path;
	s32 index = entity->DEBUG_step_count++;
	if (index < path->length)
		entity->p = path->tiles[index].p;
	#else
	int randomIndex = rand() % 4;

	v2s chosenDir;
	if (rand() % 100 < 25) {
		chosenDir = cardinal_directions[randomIndex]; //NOTE(): replaced by enemy ai
	} else {
		chosenDir = GetDirectionToClosestPlayer(World->storage, requestee->p);
	}

	int canMove = false;
	int attempts = 0;

	v2s peekPos = Add32(requestee -> p, chosenDir);

	while(!canMove && attempts < 5){
		canMove = MoveFitsWithSize(World, requestee, peekPos);
		
		if(canMove) {
			Move(World, requestee, chosenDir);
			ApplyTileEffects(requestee->p, World, requestee);
			break;
		}

		peekPos = Add32(requestee -> p, chosenDir);
		chosenDir = cardinal_directions[(randomIndex + 1 ) % 4];
		attempts++;
	}
	#endif
	return 1;
}

fn void Perish(game_world_t *game, entity_t *entity)
{
	switch (entity->enemy_type)
	{
	case enemy_slime_large:
		{
			SubdivideLargeSlime(game, entity, 0, 0);
			SubdivideLargeSlime(game, entity, 1, 0);
			SubdivideLargeSlime(game, entity, 1, 1);
			SubdivideLargeSlime(game, entity, 0, 1);
		} break;
	}
}

fn entity_id_t AttemptAttack(game_world_t *World, entity_t *requestee, s32 effective_range)
{
	turn_queue_t *queue = World->turns;
	entity_id_t result = 0;

	// TODO(): This should be like, a nearest player in range.
	entity_t *target = DEBUGGetPlayer(World->storage);
	if (target && IsInsideCircle(target->p, target->size, requestee->p, effective_range))
	{
		result = target->id;

		f32 random_chance = RandomFloat();
		DebugLog("%f", random_chance);

		if (RandomChance(30))
			queue->enemy_action = enemy_action_slash;
		else
			queue->enemy_action = enemy_action_shoot;

	}
	return result;
}

fn void RenderEntity(command_buffer_t *out, const entity_t *entity, f32 alpha, assets_t *assets, const map_t *map)
{
	v2 p = entity->deferred_p;
	bitmap_t *bitmap = IsHostile(entity) ? &assets->Slime : &assets->Player[0];
	v2 bitmap_p = p;
	// TODO(): Still somewhat hard-coded.
	v2 cube_bb_sz = V2(24, 24);
	if ((entity->size.x == 2) && (entity->size.y == 2))
	{
		bitmap = &assets->SlimeBig;
		bitmap_p = Add(bitmap_p, Scale(map->tile_sz, 0.5f));
		p = Add(p, Scale(map->tile_sz, 0.5f));
		cube_bb_sz = V2(64.0f, 64.0f);
	}
	// NOTE(): Bitmap
	v4 bitmap_color = PureWhite();
	v2 bitmap_sz = bitmap->scale;
	bitmap_p = ScreenToIso(bitmap_p);
	bitmap_p = Sub(bitmap_p, Scale(bitmap_sz, 0.5f)); //center bitmap
	bitmap_p.y -= bitmap_sz.y * 0.25f; //center bitmap "cube"

	// NOTE(): Flickering
	if (entity->blink_time > 0)
	{
		f32 t = entity->blink_time;
		t = t * t;
		f32 blink_rate = 0.4f;
		if (fmodf(t, blink_rate) >= (blink_rate * 0.5f))
			bitmap_color = Red();
		else
			bitmap_color = A(Blue(), 0.9f);
	}

	bitmap_color.w = alpha;
	DrawBitmap(out, bitmap_p, bitmap_sz, bitmap_color, bitmap);
	RenderIsoCubeCentered(out, ScreenToIso(p), cube_bb_sz, 50, Pink());
}

fn void DoEnemyAction(game_world_t *game, entity_t *entity, entity_t *target, f32 t, f32 dt, assets_t *assets, command_buffer_t *out, b32 inflict_damage)
{
	turn_queue_t *queue = game->turns;
	switch (queue->enemy_action)
	{
	case enemy_action_shoot:
		{
			if (t < 1.0f)
				DrawRangedAnimation(out, entity->deferred_p, target->deferred_p, &assets->SlimeBall, t);
			if (inflict_damage)
				DoDamage(game, target, entity->attack_dmg, "");
		} break;
	case enemy_action_slash:
		{
			v2 p = EaseInThenOut(40.0f, 60.0f, 15.0f, t);
			f32 alpha = 1.0f - p.y;

			DrawDiegeticText(game, entity->deferred_p, V2((-20.0f + p.x), -80.0f), A(White(), alpha), "SLASH!");
			if (inflict_damage)
				DoDamage(game, target, (entity->attack_dmg + 7), "");
		} break;
	}
	
}