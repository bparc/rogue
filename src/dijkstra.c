typedef struct
{
	s16 x;
	s16 y;
	s16 distance;
} flood_fill_node_t;

typedef struct
{
	map_t *map;
	s32 count;
	flood_fill_node_t nodes[1024*2];
} flood_stack_t;

fn flood_fill_node_t *Visit(flood_stack_t *stack, s32 x, s32 y, s16 distance)
{
	flood_fill_node_t *result = 0;
	if (stack->count < ArraySize(stack->nodes))
	{
		result = &stack->nodes[stack->count++];
		result->x = (s16)x;
		result->y = (s16)y;
		result->distance = distance;
		SetTileDistance(stack->map, x, y, distance);
	}
	return result;
}

fn flood_fill_node_t PopLeastDistanceNode(flood_stack_t *stack)
{
	s32 result_index = -1;
	flood_fill_node_t result = {0};
	result.distance = INT16_MAX;

	for (s32 index = 0; index < stack->count; index++)
	{
		flood_fill_node_t *node = &stack->nodes[index];
		if (node->distance < result.distance)
		{
			result_index = index;
			result = *node;
		}
	}
	
	Assert((result_index != -1));
	stack->nodes[result_index] = stack->nodes[--stack->count];
	return result;
}

#define NOT_VISITED INT16_MAX
fn inline void Flood(flood_stack_t *stack, flood_fill_node_t source, s32 x, s32 y)
{
	x = (source.x + x);
	y = (source.y + y);
	if ((GetTileDistance(stack->map, x, y) == NOT_VISITED) && IsTraversable(stack->map, x, y))
		Visit(stack, x, y, (source.distance + 1));
}

fn void ComputeDistances(map_t *map, s32 x, s32 y, memory_t memory)
{
	for (s32 index = 0; index < (map->x * map->y); index++)
		map->tiles[index].distance = NOT_VISITED;
	flood_stack_t *stack = PushStruct(flood_stack_t, &memory); Assert(stack);
	stack->map = map;
	stack->count = 0;
	Visit(stack, x, y, 0);
	while (stack->count > 0)
	{
		flood_fill_node_t top =	 PopLeastDistanceNode(stack);
		Flood(stack, top, +1, +0);
		Flood(stack, top, -1, +0);
		Flood(stack, top, +0, +1);
		Flood(stack, top, +0, -1);
	}
}
#undef NOT_VISITED

fn v2s FindNearestTile(map_t *map, v2s source)
{
	v2s result = {-1, -1};

	s32 min_distance = INT32_MAX;
	v2s at = source;
	for (s32 index = 0; index < 4; index++)
	{
		const v2s directions[4] = { V2s(-1, +0), V2s(+1, +0), V2s(+0, +1), V2s(+0, -1) };
		v2s d = directions[index];
		
		s32 x = at.x + d.x;
		s32 y = at.y + d.y;
		tile_t *neighbor = GetTile(map, x, y);
		if ((neighbor) && (neighbor->distance < min_distance))
		{
			min_distance = neighbor->distance;
			result = V2s(x, y);
		}
	}

	return result;
}