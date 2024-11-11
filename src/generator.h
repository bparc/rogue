typedef struct
{
	v2s min;
	v2s max;
	v2s ChunkAt;
	v2s PrevChunk;
} room_t;

typedef struct
{
	#define COUNT 1024
	s32 PlacedRoomCount;
	room_t PlacedRooms[COUNT];
	#undef COUNT

	b32 MarkedChunks[16][16];
	b32 OccupiedChunks[16][16];
	s32 ChunkCountX;
	s32 ChunkCountY;
} map_layout_t;

fn void SetupGenerator(map_layout_t *Gen)
{
	ZeroStruct(Gen);
	Gen->ChunkCountY = ArraySize(Gen->OccupiedChunks);
	Gen->ChunkCountX = ArraySize(Gen->OccupiedChunks[0]);
}

fn void RenderDebugGeneratorState(command_buffer_t *out, map_layout_t *Gen, v2s PlayerP, assets_t *Assets);