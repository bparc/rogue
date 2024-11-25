fn v2s GetRoomPosition(const map_layout_t *Layout, const room_t *Room)
{
	v2s result = Mul32(Room->ChunkAt, Layout->ChunkSize);
	return result;
}

fn s32 IsChunkFree(const map_layout_t *Gen, v2s At)
{
	s32 result = 0;
	if ((At.x >= 0 && At.x < Gen->ChunkCountX) &&
		(At.y >= 0 && At.y < Gen->ChunkCountY))
	{
		result = (Gen->OccupiedChunks[At.y][At.x] == false);
	}
	return result;
}

fn s32 FindRandomAdjacentChunk(map_layout_t *Gen, v2s At, v2s *Chunk)
{
	s32 random_dir = rand() % 4;

	for (s32 DirIndex = random_dir; DirIndex <= (random_dir + 4); DirIndex++)
	{
		v2s Dir = cardinal_directions[DirIndex % 4];
		v2s attempted_chunk = Add32(At, Dir);
		if (IsChunkFree(Gen, attempted_chunk))
		{
			*Chunk = attempted_chunk;
			return true;
		}
	}

	return 0;
}

fn s32 HasAnyFreeRoomLink(const map_layout_t *Gen, v2s At)
{
	for (s32 DirIndex = 0; DirIndex < 4; DirIndex++)
	{
		v2s Dir = cardinal_directions[DirIndex];
		if (IsChunkFree(Gen, Add32(At, Dir)))
			return true;
	}
	return false;
}

fn s32 FindRandomDisjointedChunk(map_layout_t *Gen, v2s *Chunk, memory_t Memory)
{
	s32 max_candidate_count = 1024;
	s32 candidate_count = 0;
	s32 *candidates = PushArray(s32, &Memory, max_candidate_count);

	for (s32 Index = 0;
		(Index < Gen->PlacedRoomCount) && (candidate_count < max_candidate_count);
		Index++)
	{
		room_t *room = &Gen->PlacedRooms[Index];
		if (HasAnyFreeRoomLink(Gen, room->ChunkAt))
			candidates[candidate_count++] = Index;
	}

	if (candidate_count > 0)
	{
		s32 RandomCandidateIndex = (rand() % candidate_count);
		s32 RoomIndex = candidates[RandomCandidateIndex];
		*Chunk = Gen->PlacedRooms[RoomIndex].ChunkAt;
		return true;
	}
	return 0;
}

fn void PushDoor(room_t *Room, v2s ChunkTo)
{
	if (Room)
	{
		if (Room->DoorCount < ArraySize(Room->Doors))
		{
			v2s DoorDir = Sub32(ChunkTo, Room->ChunkAt);
			Room->Doors[Room->DoorCount++] = DoorDir;
		}
	}
}

fn room_t *PlaceRoom(map_layout_t *Gen, v2s At, v2s PrevChunk)
{
	room_t *result = 0;
	if (Gen->PlacedRoomCount < ArraySize(Gen->PlacedRooms))
	{
		result = &Gen->PlacedRooms[Gen->PlacedRoomCount++];
		result->ChunkAt = At;
		result->PrevChunk = PrevChunk;
		result->Index = Gen->PlacedRoomCount;
		result->min = Mul32(result->ChunkAt, Gen->ChunkSize);	
		result->max = Add32(result->min, Gen->ChunkSize);

		room_t *PrevRoom = RoomFromChunkIndex(Gen, PrevChunk);
		if (PrevRoom)
		{
			PushDoor(PrevRoom, result->ChunkAt);
			PushDoor(result, PrevRoom->ChunkAt);
		}

		Gen->OccupiedChunks[At.y][At.x] = true;
		Gen->RoomIndexes[At.y][At.x] = result->Index;
	}
	return result;
}

fn void GenerateDungeon(map_layout_t *Gen, s32 RequestedRoomCount, memory_t Memory)
{
	SetupGenerator(Gen);
	u32 Seed = (u32)time(0);
	srand(Seed);

	s32 remainig_rooms = RequestedRoomCount;
	s32 max_sequence_lengt = 3;
	s32 current_room_sequence_length = 0;
	v2s chunk_at = V2S(0, 0);
	while (1)
	{
		v2s NextChunk = {0};
		if ((remainig_rooms > 0))
		{
			s32 RoomFound = 0;
			RoomFound = FindRandomAdjacentChunk(Gen, chunk_at, &NextChunk);

			if (!RoomFound || (current_room_sequence_length >= max_sequence_lengt))
			{
				RoomFound = FindRandomDisjointedChunk(Gen, &chunk_at, Memory);
				if (RoomFound)
				{
					current_room_sequence_length = 0;
					max_sequence_lengt = 3;
					RoomFound = FindRandomAdjacentChunk(Gen, chunk_at, &NextChunk);
				}
			}

			if (RoomFound)
			{
				if (PlaceRoom(Gen, NextChunk, chunk_at))
				{
					remainig_rooms--;
					current_room_sequence_length++;
					chunk_at = NextChunk;
					continue;
				}
			}
		}

		break;
	}

	DebugLog("placed %i/%i rooms, seed = %i", RequestedRoomCount - remainig_rooms, RequestedRoomCount, Seed);
}

fn v2s GenerateDoorPos(map_layout_t *Layout, v2s ChunkAt, v2s Dir)
{
	v2s chunk_size = Layout->ChunkSize;
	v2s chunk_size_half = Layout->ChunkSizeHalf;
	v2s min = Mul32(ChunkAt, Layout->ChunkSize);

	v2s edge_offset = {0};
	edge_offset.x = Dir.x < 0 ? 2 : 0;
	edge_offset.y = Dir.y < 0 ? 2 : 0;

	v2s door_relative_offset = {0};
	v2s door_dir = Dir;
	if ((door_dir.y == 0))
	{
		door_relative_offset.y = chunk_size_half.y;
		if ((door_dir.x == +1))
			door_relative_offset.x = chunk_size.x - edge_offset.x;
	}
	if ((door_dir.x == 0))
	{
		door_relative_offset.x = chunk_size_half.x;
		if ((door_dir.y == +1))
			door_relative_offset.y = chunk_size.y - edge_offset.y;
	}
	v2s Result = Add32(min, door_relative_offset);
	return Result;
}

fn void LayoutRoom(map_t *Map, room_t room, const b32 PlaceDoors, map_layout_t *Layout)
{
	v2s chunk_size = Layout->ChunkSize;
	v2s chunk_size_half = Layout->ChunkSizeHalf;
	v2s min = Mul32(room.ChunkAt, chunk_size);
	v2s max = Add32(min, chunk_size);

	min = Add32(min, V2S(1, 1));
	max = Sub32(max, V2S(1, 1));

	if (!PlaceDoors)
	{
		for (s32 y = min.y; y <= max.y; y++)
		{
			for (s32 x = min.x; x <= max.x; x++)
			{
				SetTileValue(Map, V2S(x, y), 1);	
			}
		}
	
		{
		for (s32 y = min.y; y <= max.y; y++)
		{
			SetTileValue(Map, V2S(min.x, y), 2);
			SetTileValue(Map, V2S(max.x, y), 2);
		}
		for (s32 x = min.x; x <= max.x; x++)
		{
			SetTileValue(Map, V2S(x, min.y), 2);
			SetTileValue(Map, V2S(x, max.y), 2);
		}
		}
	}
	else
	{
		for (s32 Index = 0; Index < room.DoorCount; Index++)
		{
			v2s Dir = room.Doors[Index];
			v2s Pos = GenerateDoorPos(Layout, room.ChunkAt, Dir);

			SetTileValue(Map, Pos, tile_floor);
			SetTileValue(Map, Add32(Pos, Dir), tile_door);
			SetTileValue(Map, Sub32(Pos, Dir), tile_door);

			v2s PerpDir = {Dir.y, -Dir.x};
			SetTileValue(Map, Add32(Pos, PerpDir), tile_wall);
			SetTileValue(Map, Sub32(Pos, PerpDir), tile_wall);
		}
	}
}

fn void CreateMapFromLayout(map_t *Map, map_layout_t *Layout)
{
	ClearMap(Map);
	
	for (s32 Index = 0; Index < Layout->PlacedRoomCount; Index++)
    {
        room_t *room = &Layout->PlacedRooms[Index];
        LayoutRoom(Map, *room, false, Layout);
    }
    for (s32 Index = 1; Index < Layout->PlacedRoomCount; Index++)
    {
        room_t *room = &Layout->PlacedRooms[Index];
        LayoutRoom(Map, *room, true, Layout);
    }
}

fn s32 IsMapLayoutIndexValid(map_layout_t *Layout, v2s Index)
{
	s32 result = (Index.x >= 0 && Index.y >= 0) &&
		(Index.x < Layout->ChunkCountX && Index.y < Layout->ChunkCountX);
	return result;
}

fn room_t *RoomFromIndex(map_layout_t *Layout, s32 Index)
{
	room_t *Result = 0;
	if (Index > 0)
		Result = &Layout->PlacedRooms[Index - 1];
	return Result;
}

fn room_t *RoomFromChunkIndex(map_layout_t *Layout, v2s Index)
{
	room_t *Result = 0;
	if (IsMapLayoutIndexValid(Layout, Index))
	{
		s32 RoomIndex = Layout->RoomIndexes[Index.y][Index.x];
		Result = RoomFromIndex(Layout, RoomIndex);
	}
	return Result;
}

fn room_t *RoomFromPosition(map_layout_t *Layout, v2s Pos)
{
	room_t *Result = 0;

	v2s Index = Div32(Pos, Layout->ChunkSize);
	if (IsMapLayoutIndexValid(Layout, Index))
	{
		s32 RoomIndex = Layout->RoomIndexes[Index.y][Index.x];	
		Result = RoomFromIndex(Layout, RoomIndex);
	}

	return Result;
}

fn void OpenEveryDoor(map_t *Map, const room_t *Room)
{
	if (Room)
		SetTileValue(Map, Room->Doors[0], tile_floor);
}