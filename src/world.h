#include "draw.h"
#include "draw.c"

typedef u64 entity_id_t;
#include "entity_actions.h"
#include "entity.h"
#include "entity.c"
#include "entity_actions.c"
#include "turn_system.h"

#include "cursor.h"

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

fn entity_t *CreateSlime(game_world_t *state, v2s p);
fn void CreateBigSlime(game_world_t *state, v2s p);
fn void CreatePoisonTrap(game_world_t *state, v2s p);

fn b32 IsWorldPointEmpty(game_world_t *state, v2s p);
fn b32 Move(game_world_t *world, entity_t *entity, v2s offset);
#include "world.c"

#include "game.h"
#include "game.c"
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

	// NOTE(): Map

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
					bitmap_t* bitmap = PickTileBitmap(map, x, y, assets);
					
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

	// NOTE(): Entities
	
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
		HealthBar(out, ScreenToIso(p), assets, entity);
	}
}