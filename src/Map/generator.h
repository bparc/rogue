typedef struct
{
	v2s min;
	v2s max;
	v2s ChunkAt;
	v2s PrevChunk;
	s32 Index;

	v2s Doors[1];
} room_t;

typedef struct
{
	#define COUNT 1024
	s32 PlacedRoomCount;
	room_t PlacedRooms[COUNT];
	#undef COUNT

	b32 RoomIndexes[16][16];
	b32 OccupiedChunks[16][16];
	s32 ChunkCountX;
	s32 ChunkCountY;

	v2s ChunkSize;
	v2s ChunkSizeHalf;
} map_layout_t;

fn void SetupGenerator(map_layout_t *Gen)
{
	ZeroStruct(Gen);
	Gen->ChunkCountY = ArraySize(Gen->OccupiedChunks);
	Gen->ChunkCountX = ArraySize(Gen->OccupiedChunks[0]);
	Gen->ChunkSize = V2S(20, 20);
	Gen->ChunkSizeHalf = Div32(Gen->ChunkSize, V2S(2, 2));
}

fn void OpenEveryDoor(map_t *Map, const room_t *Room);
fn room_t *RoomFromPosition(map_layout_t *Layout, v2s P);