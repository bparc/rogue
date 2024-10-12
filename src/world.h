typedef u64 entity_id_t;

typedef enum
{
	entity_flags_controllable = 1 << 0,
	entity_flags_hostile = 1 << 1,
} entity_flags_t;

typedef enum {
    status_effect_none = 0,
    status_effect_poison,
} status_effect_type_t;

typedef struct {
    status_effect_type_t type;
    s32 remaining_turns;
} status_effect_t;

#define MAX_STATUS_EFFECTS 3

typedef struct
{
	u8 flags;
	entity_id_t id;
	v2s p; // A position on the tile map.
	v2 deferred_p;

	u16 health;
	u16 attack_dmg;

	status_effect_t status_effects[MAX_STATUS_EFFECTS];

} entity_t;

typedef struct
{
	s32 num;
	entity_t entities[1024];
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
	u16 temp_attack_dmg = 40;
	CreateEntity(state->storage, V2S(10, 5), entity_flags_controllable, temp_player_health, temp_attack_dmg, state->map);
	CreateEntity(state->storage, V2S(11, 5), entity_flags_controllable, temp_player_health, temp_attack_dmg, state->map);
	//CreateEntity(state->storage, V2S(12, 5), entity_flags_controllable);
	state->camera_position = V2(0, 0);

	// Place traps on the map
    tile_t *tile;

    tile = GetTile(state->map, 12, 5);
    tile->trap_type = trap_type_physical;

    tile = GetTile(state->map, 13, 5);
    tile->trap_type = trap_type_poison;
}

fn void BeginGameWorld(game_world_t *state)
{
	DebugPrint("Player Controls: WASD; Hold shift for diagonal input.");
	DebugPrint("Cursor Controls: Press E to open the cursor. Press Q to close it.");
	DebugPrint("Moves: %i", state->turns->action_points);
}

fn void EndGameWorld(game_world_t *state)
{

}

fn void HUD(command_buffer_t *out, game_world_t *state, turn_queue_t *queue, entity_storage_t *storage, assets_t *assets)
{
	f32 y = 0.0f;
	v2 frame_sz = V2(42.f, 42.f);

	for (s32 index = queue->num - 1; index >= 0; index--)
	{
		entity_t *entity = GetEntity(storage, queue->entities[index]);
		if (entity )
		{
			bitmap_t *bitmap = IsHostile(entity) ? &assets->Slime : &assets->Player[0];			
			v4 frame_color = (index == (queue->num - 1)) ? Red() : Black();

			v2 p = V2(4.0f, 100.0f + y);
			DrawRect(out, p, frame_sz, V4(0.0f, 0.0f, 0.0f, 0.5f));
			DrawRectOutline(out, p, frame_sz, frame_color);
			if (bitmap)
				DrawBitmap(out, p, frame_sz, PureWhite(), bitmap);
		}
		y += (frame_sz.y + 5.0f);
	}
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

	s32 random_variant = 0;
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
					random_variant = SRandInt(random_variant);
					s32 bitmap_index = (random_variant % ArraySize(assets->Tiles));

					bitmap_t *bitmap = &assets->Tiles[bitmap_index];
					v2 p = MapToScreen(map, V2S(x, y));
					p = ScreenToIso(p);
					//p = Sub(p, Scale(bitmap->scale, 0.5f));
					p.x -= bitmap->scale.x * 0.5f;
					DrawBitmap(out, p, bitmap->scale, PureWhite(), bitmap);
					//DrawRectOutline(out, p, bitmap->scale, Red());
				}
				#endif

			}
		}
	}

	// NOTE(): Render entities.
	
	for (s32 index = 0; index < storage->num; index++)
	{
		entity_t *entity = &storage->entities[index];

		// NOTE(): The "deferred_p"s of the 'active' no-player entities are
		// animated directly in TurnKernel() to allow for
		// more explicit controls over the entity animation in that section of the code-base.
		if (!IsEntityActive(state->turns, storage, entity->id) || (entity->flags & entity_flags_controllable))
			entity->deferred_p = Lerp2(entity->deferred_p, GetTileCenter(state->map, entity->p), 10.0f * dt);

		v2 p = ScreenToIso(entity->deferred_p);
		v4 color = Pink();
		v2 debug_alignment = V2(0.5f, 0.70f);
		bitmap_t *bitmap = &assets->Player[0];
		if (IsHostile(entity))
		{
			color = Red();
			bitmap = &assets->Slime;
			debug_alignment = V2(0.50f, 0.50f);
		}		
		
		{
			// TODO(): I'm rendering a hard-coded
			// slime OR player bitmap here to test things out.
			// This bitmap should eventually be made into a customizable property of an
			// entity.
			v2 bitmap_sz = bitmap->scale;
			v2 bitmap_half_sz = Mul(bitmap_sz, debug_alignment);
			v2 bitmap_aligment = bitmap_half_sz;
			bitmap_aligment.y += 5.0f;

			v2 bitmap_p = Sub(p, bitmap_aligment);
			DrawBitmap(out, bitmap_p, bitmap_sz, PureWhite(), bitmap);
			if (IsEntityActive(state->turns, storage, entity->id))
				color = Blue();

			//DrawRectOutline(out, bitmap_p, bitmap_sz, Orange());

			const f32 MAX_HEALTH = 100.0f;
			f32 health_percentage = (f32)entity->health / MAX_HEALTH;
			RenderHealthBar(out, bitmap_p, health_percentage, assets);
		}

		RenderIsoCubeCentered(out, p, V2(24, 24), 50, color);
	}
}