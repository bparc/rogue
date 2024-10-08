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

fn b32 IsTileTraversable(map_t *map, s32 x, s32 y)
{
	b32 result = (GetTileValue(map, x, y) > 0);
	return result;
}

fn void SetTileValue(map_t *map, s32 x, s32 y, u8 value)
{
	tile_t *tile= GetTile(map, x, y);
	if (tile)
		tile->value = value;
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