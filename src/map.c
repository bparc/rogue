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

fn b32 IsTraversable(map_t *map, s32 x, s32 y)
{
	b32 result = (GetTileValue(map, x, y) == 1);
	return result;
}

fn b32 IsEmpty(const map_t *map, s32 x, s32 y)
{
	b32 result = (GetTileValue(map, x, y) == 0);
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

fn void SetTileDistance(map_t *map, s32 x, s32 y, s16 value)
{
	tile_t *tile = GetTile(map, x, y);
	if (tile)
		tile->distance = value;
}

fn s16 GetTileDistance(const map_t *map, s32 x, s32 y)
{
	tile_t *tile = GetTile((map_t *)map, x, y);
	return (tile ? tile->distance : 0);
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
		IsEmpty(map, offset.x + direction.x, offset.y + direction.y) &&
		IsEmpty(map, offset.x + direction.x, offset.y) &&
		IsEmpty(map, offset.x, offset.y + direction.y));
	return result;
}

fn s32 IsEdge(const map_t *map, v2s offset, s32 Index)
{
	v2s direction = cardinal_directions[Index];
	s32 result = (
		IsEmpty(map, offset.x + direction.x, offset.y + direction.y) &&
		!IsEmpty(map, offset.x + direction.y, offset.y + direction.x) &&
		!IsEmpty(map, offset.x - direction.y, offset.y - direction.x));
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