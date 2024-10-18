// NOTE(): Directions
static const v2s cardinal_directions[4] = { {0, -1}, {+1, 0}, {0, +1}, {-1, 0} };
static const v2s diagonal_directions[4] = { {-1, -1}, {1, -1}, {1, +1}, {-1, +1}};

typedef enum
{
	trap_type_none = 0,
	trap_type_physical = 1,
	trap_type_poison,
} trap_type_t;

typedef struct
{
	u8 value;
	s16 distance;
	trap_type_t trap_type;
} tile_t;

typedef struct
{
	s32 x;
	s32 y;
	tile_t *tiles;
	v2 tile_sz;
} map_t;

fn map_t *CreateMap(s32 x, s32 y, memory_t *memory, f32 tile_height);
fn void ClearMap(map_t *map);

// NOTE(): Accessors
fn tile_t *GetTile(map_t *map, s32 x, s32 y);

fn void SetTileValueI(map_t *map, s32 x, s32 y, u8 value);
fn void SetTileValue(map_t *map, v2s p, u8 value);
fn void SetTileTrapType(map_t *map, v2s p, trap_type_t type);

fn u8 GetTileValue(const map_t *map, s32 x, s32 y);
fn trap_type_t GetTileTrapType(const map_t *map, s32 x, s32 y);

fn void SetTileDistance(map_t *map, s32 x, s32 y, s16 value);
fn s16 GetTileDistance(const map_t *map, s32 x, s32 y);

// NOTE(): Tile queries
fn b32 IsTraversable(map_t *map, v2s p);
fn b32 IsEmpty(const map_t *map, v2s p);
fn b32 IsWall(const map_t *map, v2s p);
fn u8 chooseTileBitmap(map_t* map, s32 x, s32 y);

fn s32 IsCorner(const map_t *map, v2s offset, s32 Index);
fn s32 IsEdge(const map_t *map, v2s offset, s32 Index);
fn s32 DetectCorner(const map_t *map, v2s offset);
fn s32 DetectEdge(const map_t *map, v2s offset);

// NOTE(): Geometric queries
fn bb_t GetTileBounds(const map_t *map, s32 x, s32 y);
fn v2 GetTileCenter(const map_t *map, v2s p);
fn v2 MapToScreen(const map_t *map, v2s p);

// NOTE(): Pathfinding
fn tile_t *FindNearestNeighbor(map_t *map, v2s from, v2s *neighbor_index);
fn void ComputeDistances(map_t *map, s32 x, s32 y, memory_t memory);