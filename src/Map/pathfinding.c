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

fn void _Flood(const map_t *Map, u8 *visited, min_queue_t *queue, min_queue_entry_t min, s32 x, s32 y, memory_t *memory)
{
	v2s position = IntAdd(min.data->p, V2S(x, y));
	s32 index = GetTileIndex(Map, position);
	if (IsTraversable(Map, position) && !visited[index])
	{
		InsertMin(queue, (min.priority + 1), position, PushStruct(path_tile_t, memory), min.data);
		visited[index] = true;
	}
}

fn v2s GetPathTile(const path_t *Path, s32 Index)
{
	v2s Result = (Path->tiles[Clamp(Index, 0, Path->length - 1)].p);
	return Result;
}

fn s32 FindPath(const map_t *Map, v2s source, v2s dest, path_t *out, memory_t memory)
{
	DebugLog("requesting a path from %i, %i to %i, %i", source.x, source.y, dest.x, dest.y);
	u8 *visited = PushArray(u8, &memory, Map->x * Map->y);
	memset(visited, 0, Map->x * Map->y);

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
			if (CompareInts(min.data->p, source))
			{
				succeeded = true;
				break;
			}
			else
			{
				_Flood(Map, visited, queue, min, +1, +0, &memory);
				_Flood(Map, visited, queue, min, +0, +1, &memory);
				_Flood(Map, visited, queue, min, -1, +0, &memory);
				_Flood(Map, visited, queue, min, +0, -1, &memory);
			}
		}

		if (succeeded)
			AssemblePath(out, min.data);
	}


	return (succeeded );
}

fn range_map_cell_t *GetRangeMapCell(range_map_t *Map, v2s Cell)
{
	range_map_cell_t *Result = NULL;

	v2s RelativeCell = IntSub(Cell, Map->Min);
	Cell = RelativeCell;

	if ((Cell.x >= 0) && (Cell.y >= 0) && (Cell.x < Map->Size.x) && (Cell.y < Map->Size.y))
	{
		Result = &Map->Cells[Cell.y][Cell.x];
	}

	return Result;
}

fn void ExpandRange(range_map_t *Map, const map_t *Obstacles, min_queue_t *Queue, memory_t *Mem, min_queue_entry_t Parent, s32 X, s32 Y) 
{
	v2s CellIndex = IntAdd(Parent.data->p, V2S(X, Y));
	range_map_cell_t *Cell = GetRangeMapCell(Map, CellIndex);

	if (Cell && IsTraversable(Obstacles, CellIndex))
	{
		if ((ManhattanDistance(Map->From, CellIndex) < Map->MaxRange) && !Cell->Filled)
		{
			InsertMin(Queue, (Parent.priority + 1), CellIndex, PushStruct(path_tile_t, Mem), Parent.data);
			Cell->Filled = true;
		}
	}
}

fn void IntegrateRange(range_map_t *Map, const map_t *Obstacles, v2s From, memory_t Memory)
{
	ZeroStruct(Map);
	Map->Size = V2S(ArraySize(Map->Cells[0]), ArraySize(Map->Cells));

	Map->From = From;
	Map->Min = IntSub(Map->From, IntHalf(Map->Size));

	Map->MaxRange = 6;

	min_queue_t *Queue = PushStruct(min_queue_t, &Memory);
	if (Queue)
	{
		InsertMin(Queue, 0, Map->From, PushStruct(path_tile_t, &Memory), NULL);
		while (Queue->count)
		{
			min_queue_entry_t Min = ExtractMin(Queue);

			if (Map->FilledCount < ArraySize(Map->Filled))
			{
				range_map_cell_t *Filled = &Map->Filled[Map->FilledCount++];
				Filled->Filled = true;
				Filled->Cell = Min.data->p;
			}

			ExpandRange(Map, Obstacles, Queue, &Memory, Min, +1, +0);
			ExpandRange(Map, Obstacles, Queue, &Memory, Min, -1, +0);
			ExpandRange(Map, Obstacles, Queue, &Memory, Min, +0, +1);
			ExpandRange(Map, Obstacles, Queue, &Memory, Min, +0, -1);
		}
	}

	DebugLog("From: %i, %i | Range: %i", From.x, From.y, Map->MaxRange);
}

fn void ClearRangeMap(range_map_t *Map)
{
	ZeroStruct(Map);
}

fn int32_t CheckRange(range_map_t *Map, v2s CellIndex)
{
	int32_t Result = 0;
	range_map_cell_t *Cell = GetRangeMapCell(Map, CellIndex);
	if (Cell)
	{
		Result = Cell->Filled;
	}
	return Result;
}