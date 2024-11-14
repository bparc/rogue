int IsValidEntity(s32 globalX, s32 globalY, char *Data, s32 startX, s32 startY, v2s size, char entityChar) {
    s32 n = size.x;
    s32 m = size.y;

    // within bounds?
    if (startX < 0 || startY < 0 || startX + n > globalX || startY + m > globalY) {
        return false;  // Out of bounds
    }

    // is entire requested size filled with entityChar?
    for (int y = startY; y < startY + m; y++) {
        for (int x = startX; x < startX + n; x++) {
            if (Data[y * globalX + x] != entityChar) {
                return false;  // Mismatch found
            }
        }
    }

    return true; 
}

fn void MakeSimpleMap(game_world_t *world)
{
	#define X 20
	#define Y 20
	// # = Traversables
	// W = Walls
    // S = Slime
    // T = Trap
    // P = Poison
    #if 0
	char Data[Y][X] =
	{
		' ',' ',' ',' ',' ','#','#','#','#',' ','#','#','#','#','#',' ','#','#','#',' ',
		' ',' ','#','#','#',' ',' ',' ','#',' ','#','#','#','#',' ',' ','#','#','#',' ',
		' ','#','#','#','#','#',' ',' ','#',' ','#','#','#',' ',' ',' ','#','S','#',' ',
		'#','#','#','#','#','#','#',' ','#',' ','#','#','#',' ',' ',' ','#','P','#',' ',
		'#','#','#','#','#','#','#',' ','#',' ','#','#','#',' ',' ',' ','#','#','#',' ',
		'#','#','#','S','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#',' ',
		'#','#','#','#','#','#','#','#','#','T','#','#','#','#','W','#','#','#','#',' ',
		'#','S',' ','#',' ','#','#','#','#','#','#','#','#','#','W','#','#','#','#',' ',
		'#','#',' ','#',' ','#','#','#','#','#','#','#','#','#','#','W','#','#','#',' ',
		' ','#',' ','S',' ','#','#','#','#','#','#','#','#',' ',' ',' ','#','#','#',' ',
		' ','#',' ','#',' ','#','#','#','#','#','#',' ',' ',' ',' ',' ',' ','#','#',' ',
		' ',' ',' ','S',' ','#','#','T','#','#','#',' ','#','#','#','#',' ',' ',' ',' ',
		' ',' ',' ',' ','#','#','#','#','#','#','#','#','#',' ',' ','#',' ',' ',' ',' ',
		' ',' ',' ',' ','#','#','#','#','#','#',' ',' ',' ',' ',' ','#',' ',' ',' ',' ',
		' ',' ',' ',' ',' ',' ','T','T','T',' ',' ','#','#','#',' ','#',' ',' ',' ',' ',
		' ',' ','#','#','#','#','#','#','#',' ',' ','#',' ','#',' ','#',' ',' ',' ',' ',
		' ',' ',' ','#',' ',' ','#','#','#',' ',' ','#',' ','#',' ','#',' ',' ',' ',' ',
		' ',' ',' ','#',' ',' ','#','#','#',' ','#','#',' ','#','#','#',' ',' ',' ',' ',
		' ',' ',' ','#',' ',' ','#','#','#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
		' ',' ',' ','#',' ',' ','#','#','#',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
	};
	#else
	char Data[20][20] = {
	' ', ' ', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', 'W', '#', '#', '#', '#', 'S', 'W', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', 'W', '#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', 'W', '#', '#', '#', '#', '#', '#', '#', 'W', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#', 'W', ' ', ' ', ' ', ' ', ' ', ' ',
	'W', 'W', 'W', 'W', 'W', '#', '#', '#', '#', '#', '#', '#', '#', 'W', 'W', 'W', 'W', 'W', ' ', ' ',
	'W', '#', '#', '#', 'W', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', 'W', ' ', ' ',
	'W', '#', 'W', '#', 'W', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', 'W', '#', 'W', 'W', 'W',
	'W', '#', 'W', '#', 'W', '#', '#', '#', '#', '#', 'S', 'W', '#', '#', '#', 'W', '#', 'W', 'S', 'W',
	'W', '#', 'W', '#', 'W', 'p', '#', '#', '#', '#', '#', '#', '#', '#', '#', 'W', '#', 'W', '#', 'W',
	'W', '#', 'W', '#', '#', '#', '#', 'W', 'S', '#', '#', '#', '#', '#', '#', 'W', '#', 'W', '#', 'W',
	'W', 'W', 'W', 'W', 'W', '#', '#', 'W', 'W', '#', '#', '#', '#', '#', 'W', 'W', '#', 'W', '#', 'W',
	' ', ' ', ' ', ' ', 'W', '#', '#', '#', 'T', 'T', 'T', '#', '#', '#', '#', 'W', '#', '#', '#', 'W',
	' ', ' ', ' ', ' ', 'W', 'W', '#', '#', '#', '#', '#', '#', '#', '#', '#', 'W', 'W', 'W', 'W', 'W',
	' ', ' ', ' ', ' ', ' ', 'W', '#', 'S', '#', '#', '#', '#', '#', '#', '#', 'W', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', 'W', '#', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', 'W', '#', '#', '#', '#', '#', '#', '#', 'W', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', '#', 'W', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', 'W', 'S', '#', '#', '#', '#', 'W', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', 'W', 'W', 'W', 'W', 'W', 'W', 'W', ' ', ' ', ' ', ' ', ' ', ' ',
	};

	#endif
	ClearMap(world->map);

	int visited[Y][X] = {false}; //needed for compound tiles larger than 1x1

	for (s32 y = 0; y < Y; y++)
	{
		for (s32 x = 0; x < X; x++)
		{
			v2s p = V2S(x, y);
			u8 value = 1;

			if (visited[y][x])
			{
				SetTileValue(world->map, p, value);
				continue;
			};  // can only fire for >1x1

			
			switch (Data[y][x])
			{
				case 'p': CreatePlayer(world, p); break;
				case '#': value = 1; break;
				case 'W': value = 2; break;
				case 'S': CreateSlime(world, p); break;
				case 'B': 
				{
					v2s size = V2S(2,2);
					// big slimes can only be 2x2
					if (IsValidEntity(X, Y, (char *)Data, x, y, size, 'B')) {
						//exclude the whole square
						for (int j = y; j < y + 2; j++) {
							for (int i = x; i < x + 2; i++) {
								visited[j][i] = true; //dont rerun those
							}
						}
						CreateBigSlime(world, p); //big slimes are 2x2
					}
					else
					{
						printf("Invalid big slime at position (%d, %d)\n", x, y);
						visited[y][x] = true;
					}
					break;
				}
				case 'T': SetTileTrapType(world->map, V2S(x, y), trap_type_poison); break;
				case 'P': SetTileTrapType(world->map, V2S(x, y), trap_type_physical); break;
				default: value = 0; break;
			}
			SetTileValue(world->map, p, value);
		}
	}

	#undef X
	#undef Y

}

fn void RenderDebugGeneratorState(command_buffer_t *out, map_layout_t *Gen, v2s PlayerP, assets_t *Assets)
{
	//DrawRect(out, V2(0.0f, 0.0f), V2(1600.0f, 900.0f), Black());
	//

	v2 ChunkSz = V2(32.0f, 32.0f);
	v2 ChunkHalfSz = Scale(ChunkSz, 0.5f);
	v2 GlobalOffset = V2(1080.0f, 10.0f);

	v2s PlayerChunkAt = Div32(PlayerP, V2S(20, 20));

#if 1
	for (s32 y = 0; y < Gen->ChunkCountY; y++)
	{
		for (s32 x = 0; x < Gen->ChunkCountX; x++)
		{
			b32 Occupied = Gen->OccupiedChunks[y][x];

			v2 p = V2((f32)x, (f32)y);
			p = Mul(p, ChunkSz);
			p = Add(p, GlobalOffset);

			bb_t Bounds = RectToBounds(p, ChunkSz);

			v4 color = White();

			DrawRect(out, Bounds.min, Sub(Bounds.max, Bounds.min), Black());
			Bounds = Shrink(Bounds, 4.0f);
			DrawRectOutline(out, Bounds.min, Sub(Bounds.max, Bounds.min), White());
		}
	}
#endif

	for (s32 index = 0; index < Gen->PlacedRoomCount; index++)
	{
		room_t *room = &Gen->PlacedRooms[index];

		v2s Chunk = room->ChunkAt;

		v2 p = V2((f32)Chunk.x, (f32)Chunk.y);
		p = Mul(p, ChunkSz);
		p = Add(p, GlobalOffset);

		bb_t Bounds = RectToBounds(p, ChunkSz);
		Bounds = Shrink(Bounds, 2.0f);
		v4 color = Blue();
		if (CompareVectors(PlayerChunkAt, Chunk))
			color = Green();

		DrawRect(out, Bounds.min, Sub(Bounds.max, Bounds.min), color);
	}

	for (s32 index = 1; index < Gen->PlacedRoomCount; index++)
	{
		room_t *room = &Gen->PlacedRooms[index];

		v2s Chunk = room->ChunkAt;
		v2s PrevChunk = room->PrevChunk;

		v2 To = V2((f32)Chunk.x, (f32)Chunk.y);
		To = Mul(To, ChunkSz);
		To = Add(To, GlobalOffset);
		To = Add(To, ChunkHalfSz);

		v2 From = V2((f32)PrevChunk.x, (f32)PrevChunk.y);
		From = Mul(From, ChunkSz);
		From = Add(From, GlobalOffset);
		From = Add(From, ChunkHalfSz);
		
		DrawLine(out, From, To, Red());
	}
}

fn void Editor(editor_state_t *editor, game_world_t *state, command_buffer_t *out,
	const client_input_t *input, log_t *log, assets_t *assets, const virtual_controls_t *cons)
{
	if ((editor->inited == false))
	{
		DebugLog("GenerateDungeon()");
		//MakeSimpleMap(state);

		GenerateDungeon(&editor->Gen, 6, *state->memory);
		LayoutMap(&editor->Gen, state);

		editor->inited = true;
	}

	entity_t *Player = DEBUGGetPlayer(state->storage);
	RenderDebugGeneratorState(Debug.out_top, &editor->Gen, Player ? Player->p : V2S(0, 0), assets);

	v2s cursor_p = ViewportToMap(state, GetCursorOffset(input));
}