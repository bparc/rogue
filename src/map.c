fn s32 GetTileIndex32(const map_t *map, s32 x, s32 y)
{
	s32 result = (y * map->x + x);
	return result;
}

fn s32 GetTileIndex(const map_t *map, v2s p)
{
	return GetTileIndex32(map, p.x, p.y);
}

fn s32 InMapBounds(const map_t *map, v2s p)
{
	s32 result = p.x >= 0 && p.y >= 0 &&
	p.x < map->x && p.y < map->y;
	return result;
}

fn tile_t *GetTile(map_t *map, s32 x, s32 y)
{
	tile_t *result = 0;
	if ((x >= 0 && y >= 0) && (x < map->x && y < map->y))
		result = (map->tiles + (y * map->x + x));
	return result;
}

fn u8 GetTileValue(const map_t *map, s32 x, s32 y)
{
	tile_t *tile = GetTile((map_t *)map, x, y);
	return (tile ? tile->value : 0);
}

fn trap_type_t GetTileTrapType(const map_t *map, s32 x, s32 y)
{
	tile_t *result = GetTile((map_t *)map, x, y);
	return (result ? result->trap_type : trap_type_none);
}

fn b32 IsTraversable(const map_t *map, v2s p)
{
	b32 result = (GetTileValue(map, p.x, p.y) == 1);
	return result;
}

fn b32 IsEmpty(const map_t *map, v2s p)
{
	b32 result = (GetTileValue(map, p.x, p.y) == 0);
	return result;
}

fn void SetTileValueI(map_t *map, s32 x, s32 y, u8 value)
{
	tile_t *tile= GetTile(map, x, y);
	if (tile)
		tile->value = value;
}

fn void SetTileValue(map_t *map, v2s p, u8 value)
{
	SetTileValueI(map, p.x, p.y, value);
}

fn void SetTileTrapType (map_t *map, v2s p, trap_type_t type)
{
	tile_t *tile = GetTile(map, p.x, p.y);
	if (tile)
		tile->trap_type = type;
}


fn bb_t GetTileBounds(const map_t *map, s32 x, s32 y)
{
	v2 min = SV2(x, y);
	min = Mul(min, map->tile_sz);
	return RectToBounds(min, map->tile_sz);
}

fn v2 GetTileCenter(const map_t *map, v2s p)
{
	v2 result = GetCenter(GetTileBounds(map, p.x, p.y));
	return result;
}

fn map_t *CreateMap(s32 x, s32 y, memory_t *memory, f32 tile_height)
{
	map_t *result = PushStruct(map_t, memory);
	result->tile_sz = V2(tile_height, tile_height);
	result->x = x;
	result->y = y;
	result->tiles = PushArray(tile_t, memory, x * y);
	Assert(result->tiles);
	return result;
}

fn void ClearMap(map_t *map)
{
	for (s32 index = 0; index < map->x * map->y; index++)
		ZeroStruct(&map->tiles[index]);
}

fn s32 IsCorner(const map_t *map, v2s offset, s32 Index)
{
	v2s direction = diagonal_directions[Index];
	s32 result = (
		IsEmpty(map, V2S(offset.x + direction.x, offset.y + direction.y)) &&
		IsEmpty(map, V2S(offset.x + direction.x, offset.y)) &&
		IsEmpty(map, V2S(offset.x, offset.y + direction.y)));
	return result;
}

fn s32 IsEdge(const map_t *map, v2s offset, s32 Index)
{
	v2s direction = cardinal_directions[Index];
	s32 result = (
		 IsEmpty(map, V2S(offset.x + direction.x, offset.y + direction.y)) &&
		!IsEmpty(map, V2S(offset.x + direction.y, offset.y + direction.x)) &&
		!IsEmpty(map, V2S(offset.x - direction.y, offset.y - direction.x)));
	return result;
}

fn s32 DetectCorner(const map_t *map, v2s offset)
{
	for (s32 index = 0; index < 4; index++)
		if (IsCorner(map, offset, index))
			return index;
	return -1;
}

fn s32 DetectEdge(const map_t *map, v2s offset)
{
	for (s32 index = 0; index < 4; index++)
		if (IsEdge(map, offset, index))
			return index;
	return -1;
}

fn v2 MapToScreen(const map_t *map, v2s p)
{
	v2 result = SV2(p.x, p.y);
	result = Mul(result, map->tile_sz);
	return (result);
}

fn u8 PickTileBitmapType(const map_t *map, s32 x, s32 y)
{
	// check if top is a tile etc
    u8 top = !IsEmpty(map, V2S(x, y - 1));
    u8 bottom = !IsEmpty(map, V2S(x, y + 1));
    u8 left = !IsEmpty(map, V2S(x - 1, y));
    u8 right = !IsEmpty(map, V2S(x + 1, y));

     //borders: connected on three sides
    if (!top && left && right && bottom) {
        return tile_border_top;
    }
    if (!bottom && left && right && top) {
        return tile_border_bottom;
    }
    if (!left && right && top && bottom) {
        return tile_border_left;
    }
    if (!right && left && top && bottom) {
        return tile_border_right;
    }

    // corners: connected on two adjacent sides
    if (!left && top && right && !bottom) {
        return tile_corner_left;
    }
    if (right && !top && !left && bottom) {
        return tile_corner_top;
    }
    if (left && !bottom && !right && top) {
        return tile_corner_bottom;
		
    }
    if (!right && bottom && left && !top) {
        return tile_corner_right;
    }

    // single connections: connected on one side
    if (left && !right && !top && !bottom) {
        return tile_single_connect_left;
    }
    if (right && !left && !top && !bottom) {
        return tile_single_connect_right;
    }
    if (top && !left && !right && !bottom) {
        return tile_single_connect_top;
    }
    if (bottom && !left && !right && !top) {
        return tile_single_connect_bottom;
    }

    // TODO: pposite sides connected |x| and = (with a small x in the middle)
    if (left && right && !top && !bottom) {
        return tile_left_right; 
    }
    if (top && bottom && !left && !right) {
        return tile_top_bottom;
    }

    // island tile
    if (!left && !right && !top && !bottom) {
        return tile_full; 
    }

    return tile_center;
}

fn bitmap_t *PickTileBitmap(const map_t* map, s32 x, s32 y, assets_t *assets)
{
	u8 ID = PickTileBitmapType(map, x, y);
	return (&assets->Tilesets[0].LowTiles[ID][0]);
}

fn b32 IsWall(const map_t *map, v2s p)
{
	b32 result = GetTileValue(map, p.x, p.y) == 2;
	return result;
}

fn void RayCast(const map_t *map, v2s from, v2s to)
{
	v2 a = SV2S(from);
	v2 b = SV2S(to);

	v2 ray = Sub(b, a);
	v2 distance_to_edge = SV2S(Sign2(ray));
	v2s delta = Sign2(ray);

	v2s at = from;
	f32 t = 0.0f;
	while (InMapBounds(map, at) && !CompareVectors(at, to))
	{
		v2 ray_p = SV2S(from);
		
		v2 edge = Add(SV2S(at), distance_to_edge);
		v2 dt = Sub(edge, ray_p);
		dt = Div(dt, ray);
		if (dt.x < dt.y)
		{
			at.x += delta.x;
			t += dt.x;
		}
		else
		{
			at.y += delta.y;
			t += dt.y;
		}
		continue;
	}
}