#include "draw.h"
#include "draw.c"

typedef u64 entity_id_t;
#include "entity_actions.h"
#include "entity.h"
#include "entity.c"
#include "entity_actions.c"
#include "turn_system.h"

#include "cursor.h"

typedef enum
{
	particle_type_none,
	//particle_type_bitmap,
	particle_type_number,
} particle_type_t;

typedef struct
{
	v2 p;
	f32 t;
	particle_type_t type;
	s32 number;
} particle_t;

typedef struct
{
	s32 num;
	particle_t parts[64];
} particles_t;

fn particle_t *CreateParticle(particles_t *particles, v2 p, particle_type_t type)
{
	particle_t *result = 0;
	if (particles->num < ArraySize(particles->parts))
		result = &particles->parts[particles->num++];
	if (result)
	{
		ZeroStruct(result);
		result->p = p;
		result->type = type;
	}
	return result;
}

fn void CreateDamageNumber(particles_t *particles, v2 p, s32 number)
{
	particle_t *result = CreateParticle(particles, p, particle_type_number);
	result->number = number;

	v2 random_offset = {0};
	random_offset.x = RandomFloat();
	random_offset.y = RandomFloat();
	random_offset = Normalize(random_offset);
	random_offset = Scale(random_offset, 5.0f);
	result->p = Add(result->p, random_offset);
}

typedef struct
{
	assets_t *assets;
	log_t *log;
	cursor_t *cursor;
	entity_storage_t *storage;
	particles_t *particles;

	slot_bar_t slot_bar;
	turn_queue_t *turns;

	map_t *map;
	v2 camera_position;
	
	memory_t memory;
} game_world_t
;
fn v2 CameraToScreen(const game_world_t *game, v2 p);
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

fn void Setup(game_world_t *state, memory_t *memory, log_t *log, assets_t *assets)
{
	state->memory = Split(memory, MB(1));

	state->assets = assets;
	state->log = log;
	state->cursor = PushStruct(cursor_t, memory);
	state->turns = PushStruct(turn_queue_t, memory);
	state->storage = PushStruct(entity_storage_t, memory);
	state->particles = PushStruct(particles_t, memory);
	state->map = CreateMap(30, 20, memory, TILE_PIXEL_SIZE);
	
	SetupActionDataTable(memory, assets);
	DefaultActionBar(&state->slot_bar,  assets);
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

fn inline void RenderEntity(command_buffer_t *out, const entity_t *entity, f32 alpha, assets_t *assets, const map_t *map)
{
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

	bitmap_color.w = alpha;
	DrawBitmap(out, bitmap_p, bitmap_sz, bitmap_color, bitmap);
	RenderIsoCubeCentered(out, ScreenToIso(p), cube_bb_sz, 50, Pink());
}

fn void DrawFrame(game_world_t *state, command_buffer_t *out, f32 dt, assets_t *assets)
{
	map_t *map = state->map;
	entity_storage_t *storage = state->storage;
	entity_t *player = DEBUGGetPlayer(storage);
	particles_t *particles = state->particles;
	command_buffer_t *out_top = Debug.out;
	turn_queue_t *queue = state->turns;

	SetGlobalOffset(out, V2(0.0f, 0.0f));
	DrawRect(out, V2(0, 0), V2(1920, 1080), SKY_COLOR); // NOTE(): Background
	SetGlobalOffset(out, state->camera_position);

	// NOTE(): Map

	for (s32 y = 0; y < map->y; y++)
	{
		for (s32 x = 0; x < map->x; x++)
		{
			v2s at = V2S(x, y);
			//s32 x = at.x;
			//s32 y = at.y;
			if (!IsEmpty(map, at))
			{
				const tile_t *Tile = GetTile(map, x, y);
				RenderIsoTile(out, map, at, White(), false, 0);
	
				if (IsWall(map, at))
					RenderIsoTile(out, map, at, White(), true, 15);
	
				if (IsTraversable(map, at))
				{
					bitmap_t *bitmap = PickTileBitmap(map, x, y, assets);
					RenderTileAlignedBitmap(out, map, at, bitmap, PureWhite());
					
					// Draw temporary blood overlay
					if (Tile->blood)
					{
						v4 color = (Tile->blood == blood_red) ? Red() : Green();
						RenderTileAlignedBitmap(out, map, at, bitmap, A(color, 0.8f));
					}
				}
	
				if ((Tile->trap_type != trap_type_none))
					RenderTileAlignedBitmap(out, map, at, &assets->Traps[(Tile->trap_type - 1)], PureWhite());	
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
		entity->blink_time = MaxF32(entity->blink_time - (dt * 1.5f), 0.0f);

		RenderEntity(out, entity, 1.0f, assets, map);
		RenderHealth(out_top, CameraToScreen(state, entity->deferred_p), assets, entity);
	}
	for (s32 index = 0; index < queue->num_evicted_entities; index++)
	{
		evicted_entity_t *entity = &queue->evicted_entities[index];
		entity->deferred_p = Sub(entity->deferred_p, Scale(V2(10.0f, 10.0f), dt));
		RenderEntity(out, &entity->entity, entity->time_remaining, assets, map);
	}
 	// NOTE(): Particles
	for (s32 index = 0; index < particles->num; index++)
	{
		particle_t *particle = &particles->parts[index];
		particle->t += dt;
		if (particle->t < 1.0f)
		{
			switch (particle->type)
			{
			case particle_type_number:
				{	
					f32 t = particle->t;
					v4 color = White();
					color.w = (1.0f - Smoothstep(t, 0.5f));

					v2 p = CameraToScreen(state, particle->p);
					p.y -= ((50.0f * t) + (t * t * t) * 20.0f);
					p.x += (Sine(t) * 2.0f - 1.0f) * 2.0f;
					DrawFormat(out_top, assets->Font, p, color, "%i", particle->number);
				} break;
			}
			continue;
		}

		particles->parts[index--] = particles->parts[--particles->num];
	}
}