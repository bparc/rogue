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

fn room_t *PlaceRoom(map_layout_t *Gen, v2s At, v2s PrevChunk)
{
	room_t *result = 0;
	if (Gen->PlacedRoomCount < ArraySize(Gen->PlacedRooms))
	{
		result = &Gen->PlacedRooms[Gen->PlacedRoomCount++];
		result->ChunkAt = At;
		result->PrevChunk = PrevChunk;
		Gen->OccupiedChunks[At.y][At.x] = true;
	}
	return result;
}

fn void GenerateDungeon(map_layout_t *Gen, s32 RequestedRoomCount, memory_t Memory)
{
	SetupGenerator(Gen);
	srand((uint32_t)time(0));

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
				current_room_sequence_length = 0;
				max_sequence_lengt = 3 + (rand() % 6);
				RoomFound = FindRandomAdjacentChunk(Gen, chunk_at, &NextChunk);
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

	DebugLog("placed %i/%i rooms", RequestedRoomCount - remainig_rooms, RequestedRoomCount);
}

fn void LayoutRoom(map_t *Map, room_t room, v2s chunk_size, const b32 PlaceDoors)
{
	v2s chunk_size_half = Div32(chunk_size, V2S(2, 2));
	v2s min = Mul32(room.ChunkAt, chunk_size);
	v2s max = Add32(min, chunk_size);

	if (!PlaceDoors)
	{
		for (s32 y = min.y; y <= max.y; y++)
		{
			for (s32 x = min.x; x <= max.x; x++)
			{
				SetTileValue(Map, V2S(x, y), 1);	
			}
		}
	
		for (s32 y = min.y; y <= max.y; y++)
		{
			SetTileValue(Map, V2S(min.x, y), 2);
			SetTileValue(Map, V2S(min.x + chunk_size.x, y), 2);
		}
		for (s32 x = min.x; x <= max.x; x++)
		{
			SetTileValue(Map, V2S(x, min.y), 2);
			SetTileValue(Map, V2S(x, min.y + chunk_size.y), 2);
		}
	}
	else
	if (!CompareVectors(room.ChunkAt, room.PrevChunk))
	{
		v2s door_relative_offset = {0};
		v2s door_dir = Sub32(room.PrevChunk, room.ChunkAt);
		if ((door_dir.y == 0))
		{
			door_relative_offset.y = chunk_size_half.y;
			if ((door_dir.x == +1))
				door_relative_offset.x = chunk_size.x;
		}
		if ((door_dir.x == 0))
		{
			door_relative_offset.x = chunk_size_half.x;
			if ((door_dir.y == +1))
				door_relative_offset.y = chunk_size.y;
		}

		SetTileValue(Map, Add32(min, door_relative_offset), 1);
	}
}

fn void GenerateRoomInterior(game_world_t *World, room_t room, v2s ChunkSize)
{
#define X 20
#define Y 20
	char Data[Y][X] = {
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', 'W', 'W', 'W', 'W', 'W', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', 'W', '#', '#', '#', 'W', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', 'W', '#', '#', '#', 'W', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', 'W', '#', '#', '#', 'W', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', 'W', 'W', 'W', 'W', 'W', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	};

	v2s min = Mul32(room.ChunkAt, ChunkSize);
	min.x++;
	min.y++;
	for (s32 y = 1; y < Y - 1; y++)
	{
		for (s32 x = 1; x < X - 1; x++)
		{
			v2s at = min;
			at.x += x;
			at.y += y;

			u8 value = 0;
			switch (Data[y][x])
			{
			case 'W': value = 2; break;
			case '#': value = 1; break;
			}
			SetTileValue(World->map, at, value);
		}
	}
#undef X
#undef Y
}

fn void LayoutMap(map_layout_t *Layout, game_world_t *World)
{
	v2s ChunkSize = V2S(20, 20);
	map_t *Map = World->map;
	ClearMap(Map);

	for (s32 Index = 0; Index < Layout->PlacedRoomCount; Index++)
	{
		room_t *room = &Layout->PlacedRooms[Index];
		LayoutRoom(Map, *room, ChunkSize, false);
	}
	for (s32 Index = 0; Index < Layout->PlacedRoomCount; Index++)
	{
		room_t *room = &Layout->PlacedRooms[Index];
		LayoutRoom(Map, *room, ChunkSize, true);
		GenerateRoomInterior(World, *room, ChunkSize);
	}

	// 
	const room_t *StartingRoom = &Layout->PlacedRooms[rand() % Layout->PlacedRoomCount];
	v2s min = Mul32(StartingRoom->ChunkAt, ChunkSize);
	CreatePlayer(World, Add32(min, V2S(5, 5)));

	CreateSlime(World, Add32(min, V2S(9, 8)));
	CreateSlime(World, Add32(min, V2S(14, 5)));

}