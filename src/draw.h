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
	p = IsoToScreen(p);
	v2 A[4], B[4];
	MakeIsoRect(A, p.x, p.y, sz);
	MakeIsoRect(B, p.x - height, p.y - height, sz);
	DrawQuad(out, A[0], A[1], A[2], A[3], color);
	if (height != 0)
	{
		DrawQuadv(out, B, color);
		DrawQuad(out, A[3], B[3], B[2], A[2], Scale3(color, 0.8f));
		DrawQuad(out, B[2], B[1], A[1], A[2], Scale3(color, 0.8f));
	}
}

fn void RenderIsoTile(command_buffer_t *out, map_t *map, v2s offset, v4 color, s32 Filled, f32 height)
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

fn void RenderRange(command_buffer_t *out, map_t *map,v2s center, int radius, v4 color)
{
    for (s32 y = center.y - radius; y <= center.y + radius; ++y)
    {
        for (s32 x = center.x - radius; x <= center.x + radius; ++x)
        {
        	v2s target = V2S(x, y);

            if (IsInsideCircle(target, V2S(1,1), center, radius)) {
				if (GetTile(map, x, y) != 0 && IsTraversable(map, target)) { // this doesn't work, its supposed to not draw range area over grids where floor isnt placed
					if (IsLineOfSight(map, center, target)) {
						RenderIsoTile(out, map, V2S(x,y), A(color, 0.5f), true, 0);
					}
				}
            }
        }
    }
}

fn void DrawRangedAnimation(command_buffer_t *out, v2 from, v2 to, bitmap_t *bitmap, f32 t)
{
	v2 target_center = to;
	v2 bitmap_p = Lerp2(from, target_center, t);
	v2 bitmap_sz = bitmap->scale;

	bitmap_p = ScreenToIso(bitmap_p);
	bitmap_p = Sub(bitmap_p, Scale(bitmap_sz, 0.5f));
	DrawBitmap(out, bitmap_p, bitmap_sz,  PureWhite(), bitmap);
}