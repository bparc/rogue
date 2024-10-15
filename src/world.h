typedef u64 entity_id_t;

typedef enum
{
	entity_flags_controllable = 1 << 0,
	entity_flags_hostile = 1 << 1,
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

typedef struct {
    status_effect_type_t type;
    s32 remaining_turns;
	s32 damage;
} status_effect_t;

#define MAX_STATUS_EFFECTS 3

typedef struct
{
	u8 flags;
	entity_id_t id;
	v2s p; // A position on the tile map.
	v2 deferred_p;

	v2s size; //size in squares, 1x1, 2x1, 1x2, etc

	u16 health;
	u16 max_health;
	u16 attack_dmg;

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



// NOTE(): Stores the turns as an list of entity ids
// in a *reverse* order (the last turn in the queue will be executed first).

typedef enum
{
	interp_request,
	interp_transit,
	interp_attack,
	interp_accept,
} interpolator_state_t;

typedef struct
{
	s32 num;
	entity_id_t entities[64];

	// TODO(): All this stuff
	// should be cleared to zero
	// at the start of the turn.
	// Maybe we could move this out into
	// some kind of "turn state" to make this a little bit easier.
	v2s starting_p;
	interpolator_state_t interp_state;
	f32 time; // NOTE(): A variable within 0.0 to 1.0 range for interpolating values.
	s32 action_points;
	s32 turn_inited;

	f32 time_elapsed; // NOTE(): Time from the start of the turn in seconds.
} turn_queue_t;



// NOTE(): Directions
static const v2s cardinal_directions[4] = { {0, -1}, {+1, 0}, {0, +1}, {-1, 0} };
static const v2s diagonal_directions[4] = { {-1, -1}, {1, -1}, {1, +1}, {-1, +1}};

typedef struct
{
	v2s p;
	b32 active; // If active, the player input is redirected to the cursor.
} cursor_t;

typedef struct
{
	cursor_t *cursor;
	entity_storage_t *storage;

	turn_queue_t *turns;

	map_t *map;
	v2 camera_position;
} game_world_t;

#include "world.c"
#include "cursor.c"
#include "gameplay.h"
#include "turn_based.c"
#include "turn_system.c"

fn void Setup(game_world_t *state, memory_t *memory, log_t *log)
{
	state->cursor = PushStruct(cursor_t, memory);
	state->turns = PushStruct(turn_queue_t, memory);
	state->storage = PushStruct(entity_storage_t, memory);
	state->map = CreateMap(30, 20, memory, TILE_PIXEL_SIZE);

	u16 temp_player_health = 100;
	u16 temp_player_max_health = 100;
	u16 temp_attack_dmg = 40;
	CreateEntity(state->storage, V2S(10, 5), V2S(1, 1), entity_flags_controllable, temp_player_health, temp_attack_dmg, state->map, temp_player_max_health);
	//CreateEntity(state->storage, V2S(11, 5), V2S(1, 1), entity_flags_controllable, temp_player_health, temp_attack_dmg, state->map, temp_player_max_health);
	state->camera_position = V2(0, 0);
}

fn void BeginGameWorld(game_world_t *state)
{
	DebugPrint("Player Controls: WASD; Hold shift for diagonal input.");
	DebugPrint("Cursor Controls: Press E to open the cursor. Press Q to close it.");
	DebugPrint("Moves: %i", state->turns->action_points);
	DebugPrint("Ent count: %i", state->storage->num);
}

fn void EndGameWorld(game_world_t *state)
{

}

fn void HUD(command_buffer_t *out, game_world_t *state, turn_queue_t *queue, entity_storage_t *storage, assets_t *assets)
{
	f32 y = 100.0f;
	v2 frame_sz = V2(42.f, 42.f);

	for (s32 index = queue->num - 1; index >= 0; index--)
	{
		entity_t *entity = GetEntity(storage, queue->entities[index]);
		if (entity )
		{
			bitmap_t *bitmap = IsHostile(entity) ? &assets->Slime : &assets->Player[0];			
			v4 frame_color = (index == (queue->num - 1)) ? Red() : Black();

			v2 p = V2(8.0f, 100.0f + y);
			DrawRect(out, p, frame_sz, V4(0.0f, 0.0f, 0.0f, 0.5f));
			DrawRectOutline(out, p, frame_sz, frame_color);
			if (bitmap)
				DrawBitmap(out, p, frame_sz, PureWhite(), bitmap);
		}
		y += (frame_sz.y + 5.0f);
	}
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
	TurnKernel(state, state->storage, state->map, state->turns, dt, &input, cons, log, out);	
	EndGameWorld(state);
	HUD(Debug.out, state, state->turns, state->storage, assets);
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

					//hardcoded for now
					u8 tile_type = chooseTileBitmap(state, x, y);
					bitmap_t* bitmap = &assets->Tilesets[0].Tiles[tile_type][0];
					
					v2 p = MapToScreen(map, V2S(x, y));
					p = ScreenToIso(p);
					p = Sub(p, Scale(bitmap->scale, 0.5f));
					//p.x -= bitmap->scale.x * 0.5f;
					DrawBitmap(out, p, bitmap->scale, PureWhite(), bitmap);
					//DrawRectOutline(out, p, bitmap->scale, Red());
				}
				#endif

				if (GetTileTrapType(map, x, y) != trap_type_none)
				{
					RenderIsoTile(out, map, V2S(x, y), LightGrey(), true, 0);
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

		v2 bitmap_sz = bitmap->scale;
		
		bitmap_p = ScreenToIso(bitmap_p);
		bitmap_p = Sub(bitmap_p, Scale(bitmap_sz, 0.5f)); //center bitmap
		bitmap_p.y -= bitmap_sz.y * 0.25f; //center bitmap "cube"

		DrawBitmap(out, bitmap_p, bitmap_sz, PureWhite(), bitmap);


		RenderIsoCubeCentered(out, ScreenToIso(p), cube_bb_sz, 50, Pink());
		RenderHealthBar(out, ScreenToIso(p), assets, entity);
	}

	
}

