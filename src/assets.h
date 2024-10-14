typedef struct
{
	bitmap_t Slime;
	bitmap_t SlimeBig;
	bitmap_t Player[4];
	bitmap_t Tiles[2];	
	bitmap_t Traps[3];

	b32 Loaded;
} assets_t;

typedef struct {

} Tile;

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

fn b32 LoadAssets(assets_t *assets)
{
	//assets->Slime = LoadBitmap();
	assets->Loaded = true;
	Bitmap(assets, &assets->Slime, "assets/enemies/slime_small_front.png", 0, 0);
	Bitmap(assets, &assets->SlimeBig, "assets/enemies/slime_big_front.png", 0, 0);
	Bitmap(assets, &assets->Tiles[0], "assets/tiles/planet_1/low_center_1.png", 0, 0);
	Bitmap(assets, &assets->Tiles[1], "assets/tiles/planet_1/low_center_2.png", 0, 0);
	Bitmap(assets, &assets->Player[0], "assets/player/player_standing_front.png", 0, 0);

	Bitmap(assets, &assets->Traps[0], "assets/environment/trap_explosive.png", 0, 0);
	Bitmap(assets, &assets->Traps[1], "assets/environment/trap_poison.png", 0, 0);
	Bitmap(assets, &assets->Traps[2], "assets/environment/trap_spike.png", 0, 0);

	return assets->Loaded;
}