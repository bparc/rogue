fn s32 GetTileIndex32(const map_t *Map, s32 x, s32 y)
{
	s32 result = (y * Map->x + x);
	return result;
}

fn s32 GetTileIndex(const map_t *Map, v2s p)
{
	return GetTileIndex32(Map, p.x, p.y);
}

fn s32 InMapBounds(const map_t *Map, v2s p)
{
	s32 result = p.x >= 0 && p.y >= 0 &&
	p.x < Map->x && p.y < Map->y;
	return result;
}

fn tile_t *GetTile(map_t *Map, s32 x, s32 y)
{
	tile_t *result = 0;
	if ((x >= 0 && y >= 0) && (x < Map->x && y < Map->y))
		result = (Map->tiles + (y * Map->x + x));
	return result;
}

fn u8 GetTileValue(const map_t *Map, s32 x, s32 y)
{
	tile_t *tile = GetTile((map_t *)Map, x, y);
	return (tile ? tile->value : 0);
}

fn trap_type_t GetTileTrapType(const map_t *Map, s32 x, s32 y)
{
	tile_t *result = GetTile((map_t *)Map, x, y);
	return (result ? result->trap_type : trap_type_none);
}

fn b32 IsTraversable(const map_t *Map, v2s p)
{
	b32 result = (GetTileValue(Map, p.x, p.y) == 1);
	return result;
}

fn b32 IsEmpty(const map_t *Map, v2s p)
{
	b32 result = (GetTileValue(Map, p.x, p.y) == 0);
	return result;
}

fn void SetTileValueI(map_t *Map, s32 x, s32 y, u8 value)
{
	tile_t *tile= GetTile(Map, x, y);
	if (tile)
		tile->value = value;
}

fn void SetTileValue(map_t *Map, v2s p, u8 value)
{
	SetTileValueI(Map, p.x, p.y, value);
}

fn void SetTileTrapType (map_t *Map, v2s p, trap_type_t type)
{
	tile_t *tile = GetTile(Map, p.x, p.y);
	if (tile)
		tile->trap_type = type;
}


fn bb_t GetTileBounds1(const map_t *Map, s32 x, s32 y)
{
	v2 min = {(f32)x, (f32)y};
	min = Mul(min, Map->tile_sz);
	return RectBounds(min, Map->tile_sz);
}

fn bb_t GetTileBounds2(const map_t *Map, v2s TileIndex)
{
	bb_t Result = GetTileBounds1(Map, TileIndex.x, TileIndex.y);
	return Result;
}

fn v2 GetTileCenter(const map_t *Map, v2s p)
{
	v2 result = BoundsCenter(GetTileBounds1(Map, p.x, p.y));
	return result;
}

fn void SetupMap(map_t *result, s32 x, s32 y, memory_t *memory, f32 tile_height)
{
	ZeroStruct(result);
	result->tile_sz = V2(tile_height, tile_height);
	result->x = x;
	result->y = y;

	s32 TileCount = x * y;
	result->container_ids = PushArray(s32, memory, TileCount);
	result->tiles 		  = PushArray(tile_t, memory, TileCount);

	Assert(result->tiles);
}

fn void ClearMap(map_t *Map)
{
	for (s32 index = 0; index < Map->x * Map->y; index++)
		ZeroStruct(&Map->tiles[index]);
}

fn s32 IsCorner(const map_t *Map, v2s offset, s32 Index)
{
	v2s direction = diagonal_directions[Index];
	s32 result = (
		IsEmpty(Map, V2S(offset.x + direction.x, offset.y + direction.y)) &&
		IsEmpty(Map, V2S(offset.x + direction.x, offset.y)) &&
		IsEmpty(Map, V2S(offset.x, offset.y + direction.y)));
	return result;
}

fn s32 IsEdge(const map_t *Map, v2s offset, s32 Index)
{
	v2s direction = cardinal_directions[Index];
	s32 result = (
		 IsEmpty(Map, V2S(offset.x + direction.x, offset.y + direction.y)) &&
		!IsEmpty(Map, V2S(offset.x + direction.y, offset.y + direction.x)) &&
		!IsEmpty(Map, V2S(offset.x - direction.y, offset.y - direction.x)));
	return result;
}

fn s32 DetectCorner(const map_t *Map, v2s offset)
{
	for (s32 index = 0; index < 4; index++)
		if (IsCorner(Map, offset, index))
			return index;
	return -1;
}

fn s32 DetectEdge(const map_t *Map, v2s offset)
{
	for (s32 index = 0; index < 4; index++)
		if (IsEdge(Map, offset, index))
			return index;
	return -1;
}

fn v2s ScreenToMap(const map_t *Map, v2 p)
{
	v2s result = {0};
	result.x = (s32)(p.x / Map->tile_sz.x);
	result.y = (s32)(p.y / Map->tile_sz.y);
	return result;
}

fn v2 MapToScreen(const map_t *Map, v2s p)
{
	v2 result = {(f32)p.x, (f32)p.y};
	result = Mul(result, Map->tile_sz);
	return (result);
}

fn u8 PickTileBitmapType(const map_t *Map, s32 x, s32 y)
{
	// check if top is a tile etc
    u8 top = !IsEmpty(Map, V2S(x, y - 1));
    u8 bottom = !IsEmpty(Map, V2S(x, y + 1));
    u8 left = !IsEmpty(Map, V2S(x - 1, y));
    u8 right = !IsEmpty(Map, V2S(x + 1, y));

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

fn bitmap_t *PickTileBitmap(const map_t* Map, s32 x, s32 y, assets_t *assets)
{
	u8 ID = PickTileBitmapType(Map, x, y);
	return (&assets->Tilesets[0].LowTiles[ID][0]);
}

fn b32 IsDoor(const map_t *Map, v2s Index)
{
	b32 result = (GetTileValue(Map, Index.x, Index.y) == tile_door);
	return result;
}

fn b32 IsWall(const map_t *Map, v2s p)
{
	b32 result = GetTileValue(Map, p.x, p.y) == 2;
	return result;
}

fn dda_line_t BeginDDALine(const map_t *Map, v2s from, v2s to)
{
	dda_line_t result = {0};
	result.at = from;
	result.from = result.at;
	result.to = to;
	result.Map = Map;

	result.ray = Sub(ToFloat(to), ToFloat(from));
	result.distance_to_edge = Sign2(result.ray);
	result.delta = FloatSign(result.ray);

	return result;
}

fn b32 ContinueDDALine(dda_line_t *it)
{
	b32 result = (InMapBounds(it->Map, it->at) && !CompareInts(it->at, it->to));

	if (result)
	{
		v2 ray_p = ToFloat(it->from);
	
		v2 edge = Add(ToFloat(it->at), it->distance_to_edge);
		v2 dt = Sub(edge, ray_p);
		dt = Div(dt, it->ray);

		if (dt.x < dt.y)
		{
			it->at.x += it->delta.x;
			it->t += dt.x;
		}
		else
		{
			it->at.y += it->delta.y;
			it->t += dt.y;
		}
	}

	return result;
}

fn b32 IsLineOfSight(const map_t *Map, v2s from, v2s to)
{
	dda_line_t dda = BeginDDALine(Map, from, to);

	while (ContinueDDALine(&dda)) {
		if (CompareInts(dda.at, from))
			continue;
		if (IsWall(Map, dda.at) || IsDoor(Map, dda.at))
			return false;
	}

	return true;
}

fn void BloodSplatter(map_t *Map, v2s shooter_position, v2s hit_position, blood_type_t blood_type, hit_velocity_t hit_velocity)
{ 
    tile_t *initial_tile = GetTile(Map, hit_position.x, hit_position.y);
    if (initial_tile && IsTraversable(Map, hit_position)) {
        initial_tile->blood = blood_type;
    }

    v2s extended_position = IntAdd(hit_position, IntSub(hit_position, shooter_position));
    dda_line_t dda = BeginDDALine(Map, hit_position, extended_position);

    int splatter_length;
    if (hit_velocity == high_velocity) {
        splatter_length = (rand() % 3) + 1;
    } else {
        splatter_length = 0;
    }
    int trail_count = 0;

    while(ContinueDDALine(&dda) && trail_count < splatter_length) {
        if (IsTraversable(Map, dda.at)) {
            tile_t *tile = GetTile(Map, dda.at.x, dda.at.y);
            if (tile) {
                tile->blood = blood_type;
                trail_count++;
            }
        } else {
            break;
        }
    }

        if (hit_velocity == low_velocity) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (rand() % 100 < 94) {
                    continue;
                }
                v2s splatter_pos = V2S(hit_position.x + dx, hit_position.y + dy);
                if (IsTraversable(Map, splatter_pos)) {
                    tile_t *tile = GetTile(Map, splatter_pos.x, splatter_pos.y);
                    if (tile) {
                        tile->blood = blood_type;
                    }
                }
            }
        }
    }
}