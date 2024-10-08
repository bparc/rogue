typedef struct
{
	u8 value;
	s16 distance;
} tile_t;

typedef struct
{
	s32 x;
	s32 y;
	tile_t *tiles;
	v2 tile_sz;
} map_t;

fn map_t *CreateMap(s32 x, s32 y, memory_t *memory, f32 tile_height);

// NOTE(): Accessors
fn tile_t *GetTile(map_t *map, s32 x, s32 y);
fn void SetTileValue(map_t *map, s32 x, s32 y, u8 value);
fn u8 GetTileValue(const map_t *map, s32 x, s32 y);

fn void SetTileDistance(map_t *map, s32 x, s32 y, s16 value);
fn s16 GetTileDistance(const map_t *map, s32 x, s32 y);

// NOTE(): Queries
fn b32 IsTileTraversable(map_t *map, s32 x, s32 y);
fn b32 IsWall(map_t *map, v2s p);

// NOTE(): Geometric queries
fn bb_t GetTileBounds(const map_t *map, s32 x, s32 y);
fn v2 GetTileCenter(const map_t *map, v2s p);

// NOTE(): Pathfinding
fn tile_t *FindNearestNeighbor(map_t *map, v2s from, v2s *neighbor_index);
fn void ComputeDistances(map_t *map, s32 x, s32 y, memory_t memory);