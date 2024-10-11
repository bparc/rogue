typedef struct
{
	bitmap_t Slime;
	bitmap_t SlimeBig;
	bitmap_t Player[4];
	bitmap_t Tiles[2];	

	b32 Loaded;
} assets_t;

fn void Bitmap(assets_t *assets, bitmap_t *bitmap, const char *path)
{
	*bitmap = LoadBitmapFromFile(path);
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
	Bitmap(assets, &assets->Slime, "assets/Slime_1.png");
	Bitmap(assets, &assets->SlimeBig, "assets/SlimeBig_1.png");
	Bitmap(assets, &assets->Tiles[0], "assets/Tile_1.png");
	Bitmap(assets, &assets->Tiles[1], "assets/Tile_2.png");
	Bitmap(assets, &assets->Player[0], "assets/PlayerFront_1.png");
	return assets->Loaded;
}