typedef struct path_tile_t path_tile_t;

struct path_tile_t
{
	struct
	{
		path_tile_t *Parent;
	};

	union
	{
		struct
		{
			s32 x;
			s32 y;
		};

		struct
		{
			v2s p;
		};
	};
};

typedef struct
{
	s32 length;
	path_tile_t tiles[1024*1024*2];
} path_t;

fn s32 FindPath(const map_t *Map, v2s source, v2s dest, path_t *out, memory_t memory);
fn v2s GetPathTile(const path_t *Path, s32 Index);

typedef struct
{
	s32 priority;
	path_tile_t *data;
} min_queue_entry_t;

typedef struct
{
	s32 count;
	min_queue_entry_t entries[1024*1024*2];
} min_queue_t;

typedef struct
{
	uint8_t Filled;
	v2s Cell;
} range_map_cell_t;

typedef struct
{
	v2s From;
	v2s Min;

	v2s Size;
	s32 MaxRange;
	
	range_map_cell_t Cells[24][24];

	s32 FilledCount;
	range_map_cell_t Filled[24 * 24];
} range_map_t;

fn range_map_cell_t *GetRangeMapCell(range_map_t *Map, v2s Cell);
fn void IntegrateRange(range_map_t *Map, const map_t *Obstacles, v2s From, memory_t Memory);
fn int32_t CheckRange(range_map_t *Map, v2s CellIndex);