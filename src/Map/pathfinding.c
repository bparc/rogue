fn min_queue_entry_t ExtractMin(min_queue_t *queue)
{
	Assert(queue->count > 0);
	s32 priority = INT32_MAX;
	s32 min_index = -1;
	for (s32 index = 0; index < queue->count; index++)
	{
		min_queue_entry_t *entry = &queue->entries[index];
		if (entry->priority < priority)
		{
			priority = entry->priority;
			min_index = index;
		}
	}
	Assert(min_index >= 0);

	min_queue_entry_t result = queue->entries[min_index];
	queue->entries[min_index] = queue->entries[--queue->count];
	return result;
}

fn void InsertMin(min_queue_t *queue, s32 priority, v2s p, path_tile_t *data, path_tile_t *parent)
{
	Assert(data);
	Assert(queue->count < ArraySize(queue->entries));
	min_queue_entry_t *result = &queue->entries[queue->count++];
	result->priority = priority;
	result->data = data;
	result->data->Parent = parent;
	result->data->p = p;
}

fn void ExpandPath(path_t *path, v2s p)
{
	if (path->length < ArraySize(path->tiles))
		path->tiles[path->length++].p = p;
}

fn void AssemblePath(path_t *out, path_tile_t *top)
{
	Assert(top);
	out->length = 0;

	path_tile_t *At = top;
	while (At)
	{
		ExpandPath(out, At->p);
		At = At->Parent;
	}
}

fn void _Flood(const map_t *map, u8 *visited, min_queue_t *queue, min_queue_entry_t min, s32 x, s32 y, memory_t *memory)
{
	v2s position = Add32(min.data->p, V2S(x, y));
	s32 index = GetTileIndex(map, position);
	if (IsTraversable(map, position) && !visited[index])
	{
		InsertMin(queue, (min.priority + 1), position, PushStruct(path_tile_t, memory), min.data);
		visited[index] = true;
	}
}

fn s32 FindPath(const map_t *map, v2s source, v2s dest, path_t *out, memory_t memory)
{
	DebugLog("requesting a path from %i, %i to %i, %i", source.x, source.y, dest.x, dest.y);
	u8 *visited = PushArray(u8, &memory, map->x * map->y);
	memset(visited, 0, map->x * map->y);

	min_queue_t *queue = PushStruct(min_queue_t, &memory);
	queue->count = 0;

	s32 succeeded  = 0;
	min_queue_entry_t min = {0};

	if (queue && visited)
	{
		InsertMin(queue, 0, dest, PushStruct(path_tile_t, &memory), NULL);
		while (queue->count > 0)
		{
			min = ExtractMin(queue);
			if (CompareVectors(min.data->p, source))
			{
				succeeded = true;
				break;
			}
			else
			{
				_Flood(map, visited, queue, min, +1, +0, &memory);
				_Flood(map, visited, queue, min, +0, +1, &memory);
				_Flood(map, visited, queue, min, -1, +0, &memory);
				_Flood(map, visited, queue, min, +0, -1, &memory);
			}
		}

		if (succeeded)
			AssemblePath(out, min.data);
	}


	return (succeeded );
}