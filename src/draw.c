fn v2 ScreenToIso(v2 p)
{
	v2 result = {0};
	result.x = Dot(p, V2(+0.50f,-0.50f));
	result.y = Dot(p, V2(+0.25f,+0.25f));
	return result;
}

fn v2 IsoToScreen(v2 p)
{
	v2 result = {0};
	result.x = Dot(p, V2(+2.0f,+4.0f));
	result.y = Dot(p, V2(-2.0f,+4.0f));
	result = Scale(result, 0.5f);
	return result;
}

fn void MakeIsoRect(v2 points[4], f32 x, f32 y, v2 sz)
{
	v2 min = V2(x, y);
	v2 max = Add(min, sz);
	points[0] = V2(min.x, min.y);
	points[1] = V2(max.x, min.y);
	points[2] = V2(max.x, max.y);
	points[3] = V2(min.x, max.y);
	points[0] = ScreenToIso(points[0]);
	points[1] = ScreenToIso(points[1]);
	points[2] = ScreenToIso(points[2]);
	points[3] = ScreenToIso(points[3]);
}

fn void RenderIsoCube(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color)
{
	p = IsoToScreen(p);
	v2 A[4], B[4];
	MakeIsoRect(A, p.x, p.y, sz);
	MakeIsoRect(B, p.x - height, p.y - height, sz);
	DrawLineLoop(out, A, 4, color);
	DrawLineLoop(out, B, 4, color);
	DrawLine(out, A[0], B[0], color);
	DrawLine(out, A[1], B[1], color);
	DrawLine(out, A[2], B[2], color);
	DrawLine(out, A[3], B[3], color);
}

fn void RenderIsoCubeCentered(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color)
{
	p.y -= sz.y * 0.25f;
	RenderIsoCube(out, p, sz, height, color);
}

fn void RenderIsoCubeFilled(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color)
{
	//Assert((0 && "Not implemented!"));
	p = IsoToScreen(p);
	v2 A[4], B[4];
	MakeIsoRect(A, p.x, p.y, sz);
	MakeIsoRect(B, p.x - height, p.y - height, sz);
	DrawQuad(out, A[0], A[1], A[2], A[3], color);
	if (height != 0)
	{
		DrawQuadv(out, B, Red());
		DrawQuad(out, A[3], B[3], B[2], A[2], Green());
		DrawQuad(out, B[2], B[1], A[1], A[2], Blue());
	}
}

fn void RenderHealthBar(command_buffer_t *out, v2 position, assets_t *assets, entity_t *entity)
{
	// todo: Add animation when chunk of health is lost, add art asset
		f32 health_percentage = (f32)entity->health / entity->max_health;

		v2 bar_size = V2(35, 3);
		v2 bar_position = Sub(position, V2(14.0f, 29.0f));

		DrawRect(out, bar_position, bar_size, Black());
		v2 health_bar_size = V2(bar_size.x * health_percentage, bar_size.y);
		
		
		DrawRect(out, bar_position, health_bar_size, Green());
		DrawRectOutline(out, bar_position, health_bar_size, Black());
}

fn void RenderIsoTile(command_buffer_t *out, const map_t *map, v2s offset, v4 color, s32 Filled, f32 height)
{
	v2 p = MapToScreen(map, offset);
	p = ScreenToIso(p);
	if (Filled)
		RenderIsoCubeFilled(out, p, map->tile_sz, height, color);
	else
		RenderIsoCube(out, p, map->tile_sz, height, color);
}

fn void RenderIsoTileArea(command_buffer_t *out, map_t *map, v2s min, v2s max, v4 color)
{
	for (int y = min.y; y < max.y; y++)
	{
		for (int x = min.x; x < max.x; x++)
			RenderIsoTile(out, map, V2S(x,y), color, true, 0);
	}
}