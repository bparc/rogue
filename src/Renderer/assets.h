#define TILE_POSITION_COUNT 14
#define TILE_VARIATIONS_COUNT_MAX 5

typedef struct {
    bitmap_t LowTiles[TILE_POSITION_COUNT][TILE_VARIATIONS_COUNT_MAX]; //order as in enum below
	bitmap_t MidTiles[TILE_POSITION_COUNT][TILE_VARIATIONS_COUNT_MAX]; //maybe require two actions to climb?
	bitmap_t HighTiles[TILE_POSITION_COUNT][TILE_VARIATIONS_COUNT_MAX]; //order as in enum below
    u16 variation_counts[TILE_VARIATIONS_COUNT_MAX]; 
} terrain_tileset_t;

typedef struct {

	//action bar
	bitmap_t action_bar_elements[3];
	bitmap_t action_bar_full;

	bitmap_t action_bar_icons[16][1]; //icons:variations, order as in world.c
} combat_ui_t;

typedef struct
{
	bmfont_t *Font;
	bitmap_t Slime;
	bitmap_t SmallSlimeMelee;
	bitmap_t BigSlimeMelee;
	bitmap_t SlimeRangedProjectile; //16x16x

	bitmap_t SlimeBig;
	bitmap_t Player[4]; //why 4 players?
	bitmap_t PlayerGrenade;
	bitmap_t LowTiles[2];	
	bitmap_t Traps[3];
	bitmap_t SlimeBall;
	
	combat_ui_t CombatUI;
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

	tile_top_bottom,
	tile_left_right,


	
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
	Bitmap(assets, &assets->SlimeBig, "assets/enemies/slime_big_front.png", 0, 0);
	

	Bitmap(assets, &assets->SmallSlimeMelee, "assets/enemies/slime_small_front_attack.png", 0, 0);
	Bitmap(assets, &assets->BigSlimeMelee, "assets/enemies/slime_big_front_attack.png", 0, 0);
	Bitmap(assets, &assets->SlimeRangedProjectile, "assets/enemies/slime_ranged_attack.png", 0, 0); //16x16px
	
		//TODO: tiles loading (do it automatically somehow)
		//ORDER MATTERS HERE!
	Bitmap(assets, &assets->Tilesets[0].LowTiles[0][0], "assets/tiles/planet_1/low_center_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[0][1], "assets/tiles/planet_1/low_center_2.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[1][0], "assets/tiles/planet_1/low_full_1.png", 0, 0);

	Bitmap(assets, &assets->Tilesets[0].LowTiles[2][0], "assets/tiles/planet_1/low_border_top_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[3][0], "assets/tiles/planet_1/low_border_bottom_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[4][0], "assets/tiles/planet_1/low_border_left_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[5][0], "assets/tiles/planet_1/low_border_right_1.png", 0, 0);

	Bitmap(assets, &assets->Tilesets[0].LowTiles[6][0], "assets/tiles/planet_1/low_corner_top_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[7][0], "assets/tiles/planet_1/low_corner_bottom_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[8][0], "assets/tiles/planet_1/low_corner_left_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[9][0], "assets/tiles/planet_1/low_corner_right_1.png", 0, 0);

	Bitmap(assets, &assets->Tilesets[0].LowTiles[10][0], "assets/tiles/planet_1/low_three_border_no_top_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[11][0], "assets/tiles/planet_1/low_three_border_no_bottom_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[12][0], "assets/tiles/planet_1/low_three_border_no_left_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[13][0], "assets/tiles/planet_1/low_three_border_no_right_1.png", 0, 0);

	Bitmap(assets, &assets->Tilesets[0].LowTiles[14][0], "assets/tiles/planet_1/low_top_bottom_border_1.png", 0, 0);
	Bitmap(assets, &assets->Tilesets[0].LowTiles[15][0], "assets/tiles/planet_1/low_left_right_border_1.png", 0, 0);

	//combat UI
	Bitmap(assets, &assets->CombatUI.action_bar_full, "assets/ui/world/combat_action_bar_full.png", 0, 0);
	Bitmap(assets, &assets->CombatUI.action_bar_elements[0], "assets/ui/world/combat_action_bar_left.png", 0, 0);
	Bitmap(assets, &assets->CombatUI.action_bar_elements[1], "assets/ui/world/combat_action_bar_center.png",0 ,0 );
	Bitmap(assets, &assets->CombatUI.action_bar_elements[2], "assets/ui/world/combat_action_bar_right.png",0, 0);

//order matters basically everywhere
	Bitmap(assets, &assets->CombatUI.action_bar_icons[1][0], "assets/ui/world/icons/action_base_melee_attack_icon.png", 0, 0);
	Bitmap(assets, &assets->CombatUI.action_bar_icons[2][0], "assets/ui/world/icons/action_base_ranged_attack_icon.png", 0, 0);
	Bitmap(assets, &assets->CombatUI.action_bar_icons[3][0], "assets/ui/world/icons/action_base_throw_grenade_icon.png", 0, 0);
	Bitmap(assets, &assets->CombatUI.action_bar_icons[4][0], "assets/ui/world/icons/action_base_push_icon.png", 0, 0);
	Bitmap(assets, &assets->CombatUI.action_bar_icons[5][0], "assets/ui/world/icons/action_base_heal_icon.png", 0, 0);


	Bitmap(assets, &assets->Player[0], "assets/player/player_standing_front.png", 0, 0);
	Bitmap(assets, &assets->PlayerGrenade, "assets/player/grenade_projectile.png", 0, 0);

	Bitmap(assets, &assets->LowTiles[0], "assets/tiles/planet_1/low_center_1.png", 0, 0);
	Bitmap(assets, &assets->LowTiles[1], "assets/tiles/planet_1/low_center_2.png", 0, 0);

	Bitmap(assets, &assets->Traps[0], "assets/environment/trap_spike.png", 0, 0);
	Bitmap(assets, &assets->Traps[1], "assets/environment/trap_poison.png", 0, 0);
	Bitmap(assets, &assets->Traps[2], "assets/environment/trap_explosive.png", 0, 0);

	Bitmap(assets, &assets->SlimeBall, "assets/enemies/slime_ranged_attack.png", 0, 0);

	return assets->Loaded;
}