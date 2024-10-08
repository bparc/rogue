typedef struct
{
	bitmap_t Slime;
	bitmap_t SlimeBig;

	b32 Loaded;
} assets_t;

fn void Bitmap(assets_t *assets, bitmap_t *bitmap, const char *path)
{
	*bitmap = LoadBitmapFromFile(path);
	if ((bitmap->handle == 0))
		assets->Loaded = 0;
}

fn b32 LoadAssets(assets_t *assets)
{
	//assets->Slime = LoadBitmap();
	Assert(!assets->Loaded);
	Bitmap(assets, &assets->Slime, "assets/Slime_1.png");
	Bitmap(assets, &assets->SlimeBig, "assets/SlimeBig_1.png");

	return assets->Loaded;
}