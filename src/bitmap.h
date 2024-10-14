typedef struct
{
	s32 x;
	s32 y;
	u32 handle;
	v2 scale;
	v2 attachment;
} bitmap_t;

fn bitmap_t LoadBitmapFromFile(const char *path);