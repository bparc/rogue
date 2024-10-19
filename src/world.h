typedef u64 entity_id_t;

typedef enum
{
	entity_flags_controllable = 1 << 0,
	entity_flags_hostile = 1 << 1,
	entity_flags_deleted = 1 << 2,
} entity_flags_t;

typedef enum {
    status_effect_none = 1 << 0,
    status_effect_poison = 1 << 1,
	status_effect_instant_damage = 1 << 2,
} status_effect_type_t;

typedef enum {
	static_entity_flags_trap = 1 << 0,
	static_entity_flags_stepon_trigger = 1 << 1,
} static_entity_flags;

const char *action_type_t_names[] =
{
	"None",
	"Melee",
	"Ranged",
	"Throw",
	"Push",
	"Heal",
};

typedef struct {
	f32 range;
	entity_id_t target;
	v2s area_of_effect;
	s32 action_point_cost;
	f32 accuracy;
	f32 damage;
	b32 is_healing;
	b32 is_status_effect;
	status_effect_type_t status_effect;
	const char *name;
} action_params_t;

typedef enum {
	action_none = 0,
	action_melee_attack,
	action_ranged_attack,
	action_throw,
	action_push,
	action_heal_self,
} action_type_t;

typedef struct {
	action_type_t type;
	bitmap_t *icon;
	action_params_t params;
} action_t;

typedef struct {
    status_effect_type_t type;
    s32 remaining_turns;
	s32 damage;
} status_effect_t;

typedef struct {
	action_t action;
} slot_t;

#define MAX_SLOTS 9
typedef struct {
	slot_t slots[MAX_SLOTS];
	s32 selected_slot;
} slot_bar_t;

#define MAX_STATUS_EFFECTS 3
typedef struct
{
	u8 flags;
	entity_id_t id;
	v2s p; // A position on the tile map.
	v2 deferred_p;

	v2s size; //size in squares, 1x1, 2x1, 1x2, etc

	f32 blink_time;
	u16 health;
	u16 max_health;
	u16 attack_dmg;

	s32 ranged_accuracy;
	s32 melee_accuracy;
	s32 evasion;
	s32 remaining_action_points;
	b32 has_hitchance_boost;

	status_effect_t status_effects[MAX_STATUS_EFFECTS];

} entity_t;

typedef struct
{
	u8 flags;
	v2s p; // A position on the tile map.
	//wont move
	v2s size; //size in squares, 1x1, 2x1, 1x2, etc

	status_effect_t status_effects[MAX_STATUS_EFFECTS];
} static_entity_t;

typedef struct
{
	s32 num;
	s32 statics_num;
	entity_t entities[1024];
	static_entity_t static_entities[1024];
	u64 next_id;
} entity_storage_t;

#include "turn_system.h"

typedef struct
{
	v2s p;
	b32 active; // If active, the player input is redirected to the cursor.

	entity_id_t target_id;
} cursor_t;

typedef struct
{
	cursor_t *cursor;
	entity_storage_t *storage;

	slot_bar_t slot_bar;
	turn_queue_t *turns;

	map_t *map;
	v2 camera_position;

#if ENABLE_DEBUG_PATHFINDING
	u8 debug_memory[MB(4)];
#endif
} game_world_t;

// TODO(): Those should propably be eventually included
// in "client.h" for consistency's sake or something.
#include "world.c"
#include "gameplay.h"
#include "cursor.c"
#include "turn_user.c"
#include "turn_system.c"
#include "hud.c"

#define MAX_PLAYER_ACTION_POINTS 10
fn void Setup(game_world_t *state, memory_t *memory, log_t *log, assets_t *assets)
{
	state->cursor = PushStruct(cursor_t, memory);
	state->turns = PushStruct(turn_queue_t, memory);
	state->storage = PushStruct(entity_storage_t, memory);
	state->map = CreateMap(30, 20, memory, TILE_PIXEL_SIZE);
	DefaultActionBar(&state->slot_bar,  assets);

	u16 player_health = 400;
	u16 player_max_health = 400;
	u16 attack_dmg = 40;
	s32 player_accuracy = 75; // Applying this value for both melee and ranged accuracy
	s32 player_evasion = 20;
	CreateEntity(state->storage, V2S(10, 5), V2S(1, 1), entity_flags_controllable,
		player_health, attack_dmg, state->map, player_max_health, player_accuracy, player_evasion, MAX_PLAYER_ACTION_POINTS);
	state->camera_position = V2(0, 0);
}

fn void BeginGameWorld(game_world_t *state)
{
	//DebugPrint("Moves: %i", state->turns->action_points);
}

fn void EndGameWorld(game_world_t *state)
{

}

fn u8 chooseTileBitmap(game_world_t* world, s32 x, s32 y) {
	

    // check if top is a tile etc
    u8 top = !IsOutOfBounds(world, V2S(x, y - 1));
    u8 bottom = !IsOutOfBounds(world, V2S(x, y + 1));
    u8 left = !IsOutOfBounds(world, V2S(x - 1, y));
    u8 right = !IsOutOfBounds(world, V2S(x + 1, y));

   
      //borders: connected on three sides
    if (!top && left && right && bottom) {
        return tile_border_top;
    }
    if (!bottom && left && right && top) {
        return tile_border_bottom;
    }
    if (!left && right && top && bottom) {
        return tile_border_left;
    }
    if (!right && left && top && bottom) {
        return tile_border_right;
    }

    // corners: connected on two adjacent sides
    if (!left && top && right && !bottom) {
        return tile_corner_left;
    }
    if (right && !top && !left && bottom) {
        return tile_corner_top;
    }
    if (left && !bottom && !right && top) {
        return tile_corner_bottom;
		
    }
    if (!right && bottom && left && !top) {
        return tile_corner_right;
    }

    // single connections: connected on one side
    if (left && !right && !top && !bottom) {
        return tile_single_connect_left;
    }
    if (right && !left && !top && !bottom) {
        return tile_single_connect_right;
    }
    if (top && !left && !right && !bottom) {
        return tile_single_connect_top;
    }
    if (bottom && !left && !right && !top) {
        return tile_single_connect_bottom;
    }

    // TODO: pposite sides connected |x| and = (with a small x in the middle)
    if (left && right && !top && !bottom) {
        return tile_full; 
    }
    if (top && bottom && !left && !right) {
        return tile_full;
    }

    // island tile
    if (!left && !right && !top && !bottom) {
        return tile_full; 
    }

    return tile_center;

}

fn void Update(game_world_t *state, f32 dt, client_input_t input, log_t *log, assets_t *assets, virtual_controls_t cons, command_buffer_t *out)
{
	BeginGameWorld(state);
	TurnKernel(state, state->storage, state->map, state->turns, dt, &input, cons, log, out, assets);	
	EndGameWorld(state);
	HUD(Debug.out, state, state->turns, state->storage, assets, &input);
}

fn void DrawFrame(game_world_t *state, command_buffer_t *out, f32 dt, assets_t *assets)
{
	const map_t *map = state->map;
	entity_storage_t *storage = state->storage;

	SetGlobalOffset(out, V2(0.0f, 0.0f));
	DrawRect(out, V2(0, 0), V2(1920, 1080), SKY_COLOR); // NOTE(): Background
	SetGlobalOffset(out, state->camera_position);

	// NOTE(): Render tiles.

	for (s32 y = 0; y < map->y; y++)
	{
		for (s32 x = 0; x < map->x; x++)
		{
			s32 value = GetTileValue(map, x, y);
			if (value > 0)
			{
				v4 color = White();
				b32 Filled = 0;
				f32 height = 0;

				if (IsWall(state, V2S(x, y)))
				{
					Filled = true;
					height = 15;
				}

				RenderIsoTile(out, map, V2S(x, y), color, Filled, height);

				#if RENDER_TILE_BITMAPS
				if (value == 1)
				{

					u8 tile_type = chooseTileBitmap(state, x, y);
					bitmap_t* bitmap = &assets->Tilesets[0].LowTiles[tile_type][0];
					
					v2 p = MapToScreen(map, V2S(x, y));
					p = ScreenToIso(p);
					p = Sub(p, Scale(bitmap->scale, 0.5f));
					//p.x -= bitmap->scale.x * 0.5f;
					DrawBitmap(out, p, bitmap->scale, PureWhite(), bitmap);
					//DrawRectOutline(out, p, bitmap->scale, Red());
				}
				#endif
				
				#if ENABLE_DEBUG_PATHFINDING
				if (!height)
				{
					u16 distance = GetTileDistance(map, x, y);
					f32 t = (f32)distance / 10.0f;
					color = Lerp4(Red(), Blue(), t);
					color.w = 0.9f;
					RenderIsoTile(out, map, V2S(x, y), color, true, 0);
				}
				#endif
				
				if (GetTileTrapType(map, x, y) != trap_type_none)
					RenderIsoTile(out, map, V2S(x, y), LightGrey(), true, 0);

				u8 tileValue = GetTileTrapType(map, x, y);
				if ( tileValue != trap_type_none){
						//RenderIsoTile(out, map, V2S(x, y), LightGrey(), true, 0);
						bitmap_t* bitmap = &assets->Traps[tileValue - 1];
						
						v2 p = MapToScreen(map, V2S(x, y));
						p = ScreenToIso(p);
						p = Sub(p, Scale(bitmap->scale, 0.5f));
						//p.x -= bitmap->scale.x * 0.5f;
						DrawBitmap(out, p, bitmap->scale, PureWhite(), bitmap);
				}
			}
		}
	}

	//render static combat objects
	for (s32 index = 0; index < storage->statics_num; index++)
	{
		static_entity_t *entity = &storage->static_entities[index];

		v4 color = Lavender();
		bitmap_t *bitmap = &assets->Traps[1];

		v2s p = entity->p; 
		v2 bitmap_p = MapToScreen(state->map, p); 

		bitmap_p = ScreenToIso(bitmap_p);
		v2 bitmap_sz = bitmap->scale;
		bitmap_p = Sub(bitmap_p, Scale(bitmap_sz, 0.5f)); // center

		DrawBitmap(out, bitmap_p, bitmap_sz, PureWhite(), bitmap);
	}

	// NOTE(): Render entities.
	
	for (s32 index = 0; index < storage->num; index++)
	{
		entity_t *entity = &storage->entities[index];

		// NOTE(): The "deferred_p"s of the 'active' no-player entities are
		// animated directly in TurnKernel() to allow for
		// more explicit controls over the entity animation in that section of the code-base.
		if ((entity->flags & entity_flags_controllable) ||
			(IsEntityActive(state->turns, storage, entity->id) == false))
		{
			entity->deferred_p = Lerp2(entity->deferred_p, GetTileCenter(state->map, entity->p), 10.0f * dt);
		}

		v2 p = entity->deferred_p;
		bitmap_t *bitmap = IsHostile(entity) ? &assets->Slime : &assets->Player[0];
		v2 bitmap_p = p;

		// TODO(): Still somewhat hard-coded.
		v2 cube_bb_sz = V2(24, 24);
		if ((entity->size.x == 2) && (entity->size.y == 2))
		{
			bitmap = &assets->SlimeBig;
			bitmap_p = Add(bitmap_p, Scale(map->tile_sz, 0.5f));
			p = Add(p, Scale(map->tile_sz, 0.5f));
			cube_bb_sz = V2(64.0f, 64.0f);
		}

		// NOTE(): Bitmap
		v4 bitmap_color = PureWhite();
		v2 bitmap_sz = bitmap->scale;
		bitmap_p = ScreenToIso(bitmap_p);
		bitmap_p = Sub(bitmap_p, Scale(bitmap_sz, 0.5f)); //center bitmap
		bitmap_p.y -= bitmap_sz.y * 0.25f; //center bitmap "cube"

		// NOTE(): Flickering
		entity->blink_time -= dt * 1.5f;
		if (entity->blink_time <= 0)
			entity->blink_time = 0;
		if (entity->blink_time > 0)
		{
			f32 t = entity->blink_time;
			t = t * t;

			f32 blink_rate = 0.4f;
			if (fmodf(t, blink_rate) >= (blink_rate * 0.5f))
				bitmap_color = Red();
			else
				bitmap_color = A(Blue(), 0.9f);
		}

		DrawBitmap(out, bitmap_p, bitmap_sz, bitmap_color, bitmap);

		RenderIsoCubeCentered(out, ScreenToIso(p), cube_bb_sz, 50, Pink());
		RenderHealthBar(out, ScreenToIso(p), assets, entity);
	}

	
}

