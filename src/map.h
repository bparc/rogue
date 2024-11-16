typedef enum
{
	trap_type_none = 0,
	trap_type_physical = 1,
	trap_type_poison,
} trap_type_t;

typedef enum
{
    blood_none = 0,
    blood_red,
    blood_green
} blood_type_t;

typedef enum
{
	tile_Floor = 1,
	tile_Door = 16,
} tile_type_t;

typedef struct
{
	u8 value;
	u8 trap_type;
	u8 blood;
} tile_t;

typedef struct
{
	union
	{
		struct { s32 x, y; };
		v2s size;
	};
	s32 *container_ids;
	tile_t *tiles;

	v2 tile_sz;
} map_t;

fn map_t *CreateMap(s32 x, s32 y, memory_t *memory, f32 tile_height);
fn void ClearMap(map_t *map);

// NOTE(): Accessors
fn s32 GetTileIndex32(const map_t *map, s32 x, s32 y);
fn s32 GetTileIndex(const map_t *map, v2s p);
fn s32 InMapBounds(const map_t *map, v2s p);

fn tile_t *GetTile(map_t *map, s32 x, s32 y);

fn void SetTileValueI(map_t *map, s32 x, s32 y, u8 value);
fn void SetTileValue(map_t *map, v2s p, u8 value);
fn void SetTileTrapType(map_t *map, v2s p, trap_type_t type);

fn u8 GetTileValue(const map_t *map, s32 x, s32 y);
fn trap_type_t GetTileTrapType(const map_t *map, s32 x, s32 y);

// NOTE(): Tile Type Queries
fn b32 IsTraversable(const map_t *map, v2s p);
fn b32 IsEmpty(const map_t *map, v2s p);

fn b32 IsWall(const map_t *map, v2s p);
fn s32 IsCorner(const map_t *map, v2s offset, s32 Index);
fn s32 IsEdge(const map_t *map, v2s offset, s32 Index);
fn s32 DetectCorner(const map_t *map, v2s offset);
fn s32 DetectEdge(const map_t *map, v2s offset);

// NOTE(): Geometric Queries
fn v2s ScreenToMap(const map_t *map, v2 p);
fn v2  MapToScreen(const map_t *map, v2s p);

fn bb_t GetTileBounds(const map_t *map, s32 x, s32 y);
fn v2 GetTileCenter(const map_t *map, v2s p);
fn void RayCast(const map_t *map, v2s from, v2s to);

// NOTE(): Assets
fn bitmap_t *PickTileBitmap(const map_t* map, s32 x, s32 y, assets_t *assets);

// NOTE(): Iterators

typedef struct
{
	const map_t *map;
	v2s from;
	v2s to;

	f32 t;
	v2s at;

	v2s delta;
	v2 ray;
	v2 distance_to_edge;
} dda_line_t;

fn dda_line_t BeginDDALine(const map_t *map, v2s from, v2s to);
fn b32 ContinueDDALine(dda_line_t *it);