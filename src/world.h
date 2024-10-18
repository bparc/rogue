typedef enum {
    status_effect_none = 1 << 0,
    status_effect_poison = 1 << 1,
	status_effect_instant_damage = 1 << 2,
} status_effect_type_t;

typedef enum {
	action_none = 0,
	action_melee_attack,
	action_ranged_attack,
	action_throw,
	action_push,
	action_heal_self,
} action_type_t;

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
    status_effect_type_t type;
    s32 remaining_turns;
	s32 damage;
} status_effect_t;

#define MAX_STATUS_EFFECTS 3

typedef struct {
	action_type_t action;
	bitmap_t *icon;
} slot_t;

#define MAX_SLOTS 9

typedef struct {
	slot_t slots[MAX_SLOTS];
	s32 selected_slot;
} slot_bar_t;

#include "entity.h"
#include "entity.c"

<<<<<<< Updated upstream
	v2s size; //size in squares, 1x1, 2x1, 1x2, etc

	f32 blink_time;
	u16 health;
	u16 max_health;
	u16 attack_dmg;

	s32 ranged_accuracy;
	s32 melee_accuracy;
	s32 evasion;

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

typedef enum
{
	interp_request,
	interp_transit,
	interp_attack,
	interp_accept,
	interp_wait_for_input,
} interpolator_state_t;

const char *interpolator_state_t_names[] = {
	"Request",
	"Transit",
	"Attack",
	"Accept",
	"Break",
};

typedef struct
{
	b32 blocking;
	action_type_t type;

	entity_id_t target_id;
	v2s target_p;
	
	f32 t;
} async_action_t;

typedef struct
{
	// NOTE(): Stores the turns as an list of entity ids
// in a *reverse* order (the last turn in the queue will be executed first).
	s32 num;
	entity_id_t entities[64];
	entity_storage_t *storage;

	// TODO(): All this stuff
	// should be cleared to zero
	// at the start of the turn.
	// Maybe we could move this out into
	// some kind of "turn state" to make this a little bit easier.
	v2s starting_p;
	entity_id_t attack_target;
	interpolator_state_t interp_state;
	f32 time; // NOTE(): A variable within 0.0 to 1.0 range for interpolating values.
	s32 action_points;
	s32 turn_inited;

	f32 seconds_elapsed; // NOTE(): Seconds elapsed from the start of the turn.

	s32 break_mode_enabled;
	interpolator_state_t requested_state;
	s32 request_step;

	v2 focus_p;

	s32 action_count;
	async_action_t actions[1];

	// NOTE(): Prev turn info.
	entity_id_t prev_turn_entity;
} turn_queue_t;

fn void QueryAsynchronousAction(turn_queue_t *queue, action_type_t type, entity_t *target, v2s target_p)
{
	async_action_t *result = 0;
	if ((queue->action_count < ArraySize(queue->actions)))
	{
		result = &queue->actions[queue->action_count++];
	}
	if (result)
	{
		ZeroStruct(result);
		if (target)
		{
			result->target_id = target->id;
			result->target_p = target->p;
		}
		else
		{
			result->target_p = target_p;
		}
		result->type = type;
	}
}

typedef struct
{
	v2s p;
	b32 active; // If active, the player input is redirected to the cursor.
} cursor_t;
=======
#include "turn_system.h"
#include "cursor.h"
>>>>>>> Stashed changes

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
#include "draw.h"
#include "draw.c"
#include "world.c"
#include "cursor.c"
#include "turn_based.c"
#include "turn_system.c"
#include "hud.c"

fn void Setup(game_world_t *state, memory_t *memory, log_t *log)
{
	state->cursor = PushStruct(cursor_t, memory);
	state->turns = PushStruct(turn_queue_t, memory);
	state->storage = PushStruct(entity_storage_t, memory);
	state->map = CreateMap(30, 20, memory, TILE_PIXEL_SIZE);
	DefaultActionBar(&state->slot_bar);

	u16 player_health = 400;
	u16 player_max_health = 400;
	u16 attack_dmg = 40;
	s32 player_accuracy = 75; // Applying this value for both melee and ranged accuracy
	s32 player_evasion = 20;
	CreateEntity(state->storage, V2S(10, 5), V2S(1, 1), entity_flags_controllable,
		player_health, attack_dmg, state->map, player_max_health, player_accuracy, player_evasion);
	state->camera_position = V2(0, 0);
}

fn void BeginGameWorld(game_world_t *state)
{
	//DebugPrint("Moves: %i", state->turns->action_points);
}

fn void EndGameWorld(game_world_t *state)
{

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

				if (IsWall(map, V2S(x, y)))
				{
					Filled = true;
					height = 15;
				}

				RenderIsoTile(out, map, V2S(x, y), color, Filled, height);

				#if RENDER_TILE_BITMAPS
				if (value == 1)
				{

					u8 tile_type = chooseTileBitmap(state->map, x, y);
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
				bitmap_color = SetAlpha(Blue(), 0.9f);
		}

		DrawBitmap(out, bitmap_p, bitmap_sz, bitmap_color, bitmap);

		RenderIsoCubeCentered(out, ScreenToIso(p), cube_bb_sz, 50, Pink());
		RenderHealthBar(out, ScreenToIso(p), assets, entity);
	}

	
}