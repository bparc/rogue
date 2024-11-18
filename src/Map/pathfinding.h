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