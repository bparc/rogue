// NOTE(): Isometric.
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

fn v2 MapToScreen(const map_t *map, v2s p)
{
	v2 result = SV2(p.x, p.y);
	result = Mul(result, map->tile_sz);
	return (result);
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
	v2 R1[4], R2[4];
	MakeIsoRect(R1, p.x, p.y, sz);
	MakeIsoRect(R2, p.x - height, p.y - height, sz);
	DrawLineLoop(out, R1, 4, color);
	DrawLineLoop(out, R2, 4, color);
	DrawLine(out, R1[0], R2[0], color);
	DrawLine(out, R1[1], R2[1], color);
	DrawLine(out, R1[2], R2[2], color);
	DrawLine(out, R1[3], R2[3], color);
}

fn void RenderIsoCubeFilled(command_buffer_t *out, v2 p, v2 sz, f32 height, v4 color)
{
	Assert("Not implemented!");
}

// NOTE(): Entities.
fn entity_t *CreateEntity(entity_storage_t *storage, v2s p, s32 flags)
{
	entity_t *result = 0;
	if (storage->num < ArraySize(storage->entities))
		result = &storage->entities[storage->num++];
	if (result)
	{
		ZeroStruct(result);
		result->p = p;
	}
	return result;
}