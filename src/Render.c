fn v2s ViewportToMap(const game_state_t *World, v2 p)
{
	p = Sub(p, World->Camera->p);
	p = IsoToScreen(p);
	p = Div(p, World->Map->tile_sz);
	v2s result = {0};
	result.x = (s32)p.x;
	result.y = (s32)p.y;
	return result;
}

fn inline void RenderEntity(command_buffer_t *out, const entity_t *entity, f32 alpha, assets_t *assets, const map_t *Map)
{
	v2 p = entity->deferred_p;
	bitmap_t *bitmap = IsHostile(entity) ? &assets->Slime : &assets->Player[0];
	v2 bitmap_p = p;
	// TODO(): Still somewhat hard-coded.
	v2 cube_bb_sz = V2(24, 24);
	if ((entity->size.x == 2) && (entity->size.y == 2))
	{
		bitmap = &assets->SlimeBig;
		bitmap_p = Add(bitmap_p, Scale(Map->tile_sz, 0.5f));
		p = Add(p, Scale(Map->tile_sz, 0.5f));
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

fn inline void RenderTile(command_buffer_t *out, map_t *Map, s32 x, s32 y, assets_t *assets, game_state_t *World)
{
	v2s at = {x, y};
	if (!IsEmpty(Map, at))
	{
		const tile_t *Tile = GetTile(Map, x, y);
		RenderIsoTile(out, Map, at, White(), false, 0);

		if (IsTraversable(Map, at))
		{
			bitmap_t *bitmap = PickTileBitmap(Map, x, y, assets);
			RenderTileAlignedBitmap(out, Map, at, bitmap, PureWhite());
			
			// Draw temporary blood overlay
			if (Tile->blood)
			{
				v4 color = (Tile->blood == blood_red) ? Red() : Green();
				RenderTileAlignedBitmap(out, Map, at, bitmap, A(color, 0.8f));
			}
		}
		if (IsWall(Map, at))
			RenderIsoTile(out, Map, at, White(), true, 15);
		if (GetTileValue(Map, at.x, at.y) == tile_door)
			RenderIsoTile(out, Map, at, Red(), true, 25);

		if ((GetContainer(World, at) != NULL))
			RenderIsoTile(out, Map, at, Green(), true, 20);

		if ((Tile->trap_type != trap_type_none))
			RenderTileAlignedBitmap(out, Map, at, &assets->Traps[(Tile->trap_type - 1)], PureWhite());	
	}
}

fn inline void Render_ClipToViewport(game_state_t *World, map_t *Map, bb_t clipplane, command_buffer_t *out)
{
	v2 viewport = Sub(clipplane.max, clipplane.min);
	assets_t *assets = World->assets;

	// NOTE(): Tessalate the clipplane into a parallelogram and two
	// triangles.
	v2s top_corner = ViewportToMap(World, TopMaxCorner(clipplane));
	v2s bot_corner = ViewportToMap(World, BotMinCorner(clipplane));

	f32 projection_coefficient = 0.5f;
	viewport = Ratio(viewport, 1.0f / projection_coefficient);

	v2s min1 = ViewportToMap(World, clipplane.min);
	v2s max1 = ViewportToMap(World, Add(clipplane.min, viewport));

	v2s min2 = ViewportToMap(World, clipplane.max);
	v2s max2 = ViewportToMap(World, Sub(clipplane.max, viewport));

	#if 0
	DrawRectOutline(Debug.out_top, clipplane.min, Sub(clipplane.max, clipplane.min), Orange());
	DrawLine(Debug.out_top, clipplane.min, Add(clipplane.min, viewport), Red());
	DrawLine(Debug.out_top, clipplane.max, Sub(clipplane.max, viewport), Red());
	#endif

	// NOTE(): Sort by Y
	v2s top_min;
    v2s top_max;
    v2s bot_min;
    v2s bot_max;
    if(min1.y<min2.y)
    {
        top_min = min1;
        top_max = max1;
        bot_min = max2;
        bot_max = min2;
    }
    else
    {
        top_min = max2;
        top_max = min2;
        bot_min = min1;
        bot_max = max1;
    }

    // NOTE(): Top triangle
    v2s min = top_min;
    v2s max = top_max;
    v2s corner = top_corner;
    for(s32 y = 0; y < min.y - corner.y; y++)
    for(s32 x = -y; x <= y; x++)
            RenderTile(out,Map,corner.x+x,corner.y+y,assets,World);
    // NOTE(): Parallelogram
    if(min1.y<min2.y)
    {
        for(s32 y = 0; y <= min2.y - min.y; y++)
        for(s32 x = 0; x <= max.x-min.x;x++)
        	RenderTile(out, Map,min.x+x+y,min.y+y,assets,World);
    }
    else
    {
        for(s32 y = 0; y <= min1.y - min.y; y++)
        for(s32 x = 0; x <= max.x-min.x;x++)
           RenderTile(out,Map,(min.x+x)-y,min.y+y,assets,World);
    }
    // NOTE(): Bot triangle
    min = bot_min;
    max = bot_max;
    corner = bot_corner;
    for(s32 y = 1; y < (corner.y - max.y); y++)
    for(s32 x = y; x < (max.x - min.x)-y; x++)
    	RenderTile(out,Map,min.x+x,min.y+y,assets,World);
}

fn void Render_DrawFrame(game_state_t *state, command_buffer_t *out, f32 dt, assets_t *assets, v2 viewport)
{
	map_t *Map = state->Map;
	entity_storage_t *storage = state->storage;
	entity_t *player = DEBUGGetPlayer(storage);
	particles_t *particles = state->particles;
	command_buffer_t *out_top = Debug.out;
	turn_system_t *queue = state->System;

	//DrawRect(out, V2(0.0f, 0.0f), V2(1000.0f, 1000.0f), SKY_COLOR); // NOTE(): Background

	bb_t view = {0.0f};
	view.min = V2(0.0f, 0.0f);
	view.max = Add(view.min, viewport);
	view = ShrinkBounds(view, 64.0f * 3.0f);

	Render_ClipToViewport(state, Map, view, out);

	// NOTE(): Entities
	for (s32 index = 0; index < storage->EntityCount; index++)
	{
		entity_t *entity = &storage->entities[index];

		// NOTE(): The "deferred_p"s of the 'active' no-player entities are
		// animated directly in TurnKernel() to allow for
		// more explicit controls over the entity animation in that section of the code-base.
		if ((entity->flags & entity_flags_controllable) || (IsActive(state->System, entity->id) == false))
		{
			entity->deferred_p = Lerp2(entity->deferred_p, GetTileCenter(state->Map, entity->p), 10.0f * dt);
		}
		entity->blink_time = MaxF32(entity->blink_time - (dt * 1.5f), 0.0f);

		RenderEntity(out, entity, 1.0f, assets, Map);

		v2 screen_p = CameraToScreen(state->Camera, Sub(entity->deferred_p, V2(60.0f, 60.0f)));
		screen_p.x -= 25.0f;
		RenderHealthPoints(out_top, screen_p, assets, entity);
	}
	for (s32 index = 0; index < queue->num_evicted_entities; index++)
	{
		evicted_entity_t *entity = &queue->evicted_entities[index];
		entity->deferred_p = Sub(entity->deferred_p, Scale(V2(10.0f, 10.0f), dt));
		RenderEntity(out, &entity->entity, entity->time_remaining, assets, Map);
	}
 	// NOTE(): Particles
	for (s32 index = 0; index < particles->num; index++)
	{
		particle_t *particle = &particles->parts[index];
		particle->t += dt;
		if (particle->t < 1.0f)
		{
			f32 t = particle->t;
			v4 color = White();
			color.w = (1.0f - Smoothstep(t, 0.5f));
			v2 p = CameraToScreen(state->Camera, particle->p);
			p.y -= ((50.0f * t) + (t * t * t) * 20.0f);
			p.x += (Sine(t) * 2.0f - 1.0f) * 2.0f;

			switch (particle->type)
			{
			case particle_type_number:
				{	
					DrawFormat(out_top, assets->Font, p, color, "%i", particle->number);
				} break;
			case particle_type_combat_text:
				{
	                const char *text = "";
	                switch (particle->combat_text)
	                {
	                    case combat_text_critical: color = Yellow(); text = "CRITICAL"; break;
	                    case combat_text_hit:      text = "HIT";      break;
	                    case combat_text_miss:     color = LightGrey(); text = "MISS";     break;
	                    case combat_text_graze:    text = "GRAZE";    break;
	                    case combat_text_alerted:  text = "!"; color = White(); break;
	                    case combat_text_heal:     text = "HEAL"; color = Green(); break;
	                }

	                DrawFormat(out_top, assets->Font, p, color, "%s", text);
				} break;
			}
			continue;
		}

		particles->parts[index--] = particles->parts[--particles->num];
	}
}