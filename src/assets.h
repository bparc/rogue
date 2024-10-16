#define TILE_POSITION_COUNT 14
#define TILE_VARIATIONS_COUNT_MAX 5

typedef struct {
    bitmap_t Tiles[TILE_POSITION_COUNT][TILE_VARIATIONS_COUNT_MAX]; //order as in enum below
    u16 variation_counts[TILE_VARIATIONS_COUNT_MAX]; 
} terrain_tileset_t;

typedef struct
{
	bmfont_t *Font;
	bitmap_t Slime;
	bitmap_t SlimeBig;
	bitmap_t Player[4];
	bitmap_t Tiles[2];	
	bitmap_t Traps[3];

	terrain_tileset_t Tilesets[1]; //replace with macro PLANETS_MAX_VARIATIONS or something

	b32 Loaded;
} assets_t;




typedef enum {
	tile_center = 0, //all 4 cardinal surrounding tiles connect to this
	tile_full,

	//three sides to connect with one being the border _direction
	tile_border_top,
	tile_border_bottom,
	tile_border_left,
	tile_border_right,

	//two sides connect, two borders facing _direction
	tile_corner_top,
	tile_corner_bottom,
	tile_corner_left,
	tile_corner_right,

	//one side connects at _direction, three are unconnected
	tile_single_connect_top,
	tile_single_connect_bottom,
	tile_single_connect_left,
	tile_single_connect_right,


	
} tile_position;

fn void Bitmap(assets_t *assets, bitmap_t *bitmap, const char *path, f32 align_x, f32 align_y)
{
	*bitmap = LoadBitmapFromFile(path);
	bitmap->attachment = V2(align_x, align_y);
	if ((bitmap->handle == 0))
	{
		assets->Loaded = 0;
		Error("Coudn't load a bitmap: %s (Make sure that the \"assets/\" folder is in the working directory)", path);
	}
}

fn b32 LoadAssets(assets_t *assets, bmfont_t *font)
{
	//assets->Slime = LoadBitmap();
	assets->Loaded = true;

	if (font)
		assets->Font = font;
	
	Bitmap(assets, &assets->Slime, "assets/enemies/slime_small_front.png", 0, 0);
	Bitmap(assets, &assets->SlimeBig, "assets/enemies/slime_big_front.png", 0, 0);	//TODO: tiles loading (do it automatically somehow)
	Bitmap(assets, &assets->Tilesets[0].Tiles[0][0], "assets/tiles/planet_1/low_center_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].Tiles[0][1], "assets/tiles/planet_1/low_center_2.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].Tiles[1][0], "assets/tiles/planet_1/low_full_1.png", 0, 0);

	Bitmap(assets, &assets->Tilesets[0].Tiles[2][0], "assets/tiles/planet_1/low_border_top_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].Tiles[3][0], "assets/tiles/planet_1/low_border_bottom_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].Tiles[4][0], "assets/tiles/planet_1/low_border_left_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].Tiles[5][0], "assets/tiles/planet_1/low_border_right_1.png", 0, 0);

	Bitmap(assets, &assets->Tilesets[0].Tiles[6][0], "assets/tiles/planet_1/low_corner_top_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].Tiles[7][0], "assets/tiles/planet_1/low_corner_bottom_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].Tiles[8][0], "assets/tiles/planet_1/low_corner_left_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].Tiles[9][0], "assets/tiles/planet_1/low_corner_right_1.png", 0, 0);

	Bitmap(assets, &assets->Tilesets[0].Tiles[10][0], "assets/tiles/planet_1/low_three_border_no_top_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].Tiles[11][0], "assets/tiles/planet_1/low_three_border_no_bottom_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].Tiles[12][0], "assets/tiles/planet_1/low_three_border_no_left_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].Tiles[13][0], "assets/tiles/planet_1/low_three_border_no_right_1.png", 0, 0);


	Bitmap(assets, &assets->Player[0], "assets/player/player_standing_front.png", 0, 0);
	Bitmap(assets, &assets->Tiles[0], "assets/tiles/planet_1/low_center_1.png", 0, 0);
	Bitmap(assets, &assets->Tiles[1], "assets/tiles/planet_1/low_center_2.png", 0, 0);

	Bitmap(assets, &assets->Traps[0], "assets/environment/trap_explosive.png", 0, 0);
	Bitmap(assets, &assets->Traps[1], "assets/environment/trap_explosive.png", 0, 0);
	Bitmap(assets, &assets->Traps[2], "assets/environment/trap_spike.png", 0, 0);

	return assets->Loaded;
}