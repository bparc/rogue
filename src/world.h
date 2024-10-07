typedef u64 entity_id_t;

typedef enum
{
	entity_flags_controllable = 1 << 1,
} 
entity_flags_t;

typedef struct
{
	u8 flags;
	entity_id_t id;
	v2s p; // A position on the tile map.
	v2 deferred_p;
} entity_t;

typedef struct
{
	s32 num;
	entity_t entities[1024];
	u64 next_id;
} entity_storage_t;

typedef struct
{
	s32 num;
	entity_id_t entities[64];
} turn_queue_t;

#include "settings.h"
typedef struct
{
	v2 entity_size;
	entity_storage_t *storage;
	turn_queue_t *turns;

	map_t *map;
	f32 time_elapsed;

	v2 camera_offset;
} game_world_t;

#include "world.c"

fn void Setup(game_world_t *world, memory_t *memory);
fn void Update(game_world_t *state, f32 dt, client_input_t input);
fn void DrawFrame(game_world_t *state, command_buffer_t *out, f32 dt);

fn void Setup(game_world_t *state, memory_t *memory)
{
	state->turns = PushStruct(turn_queue_t, memory);
	state->storage = PushStruct(entity_storage_t, memory);
	state->map = CreateMap(20, 20, memory, TILE_PIXEL_SIZE);
	state->entity_size = V2(ENTITY_SIZE, ENTITY_SIZE);
	state->camera_offset = V2(800.0f, 0.0f);

	CreateEntity(state->storage, V2s(5, 5), entity_flags_controllable);
	CreateEntity(state->storage, V2s(8, 4), 0);
	CreateEntity(state->storage, V2s(2, 12), entity_flags_controllable);
	CreateEntity(state->storage, V2s(4, 2), 0);
}

fn void Update(game_world_t *state, f32 dt, client_input_t input)
{
	map_t *map = state->map;
	entity_storage_t *storage = state->storage;
	turn_queue_t *turns = state->turns;
	if (turns->num == 0)
	{
		// NOTE(): We run out of the turns, time to schedule new ones.
		DefaultTurnOrder(turns, storage);
		// NOTE():  Maybe a new turn should be scheduled in *immediately* after the
		// current one ends?
	}

	if (turns->num > 0)
	{
		// NOTE(): Process the turn
		entity_t *entity = NextInOrder(turns, storage);
		if (entity)
		{
			// NOTE(): The turn will "stall" until AcceptTurn() is called.
			if (entity->flags & entity_flags_controllable)
			{
				// NOTE(): We propably want to render stuff like this from here even
				// when NOT in the debug mode.
				#if _DEBUG
				RenderIsoTile(Debug.out, map, entity->p, state->camera_offset, Green());
				#endif

				v2s direction = GetDirectionalInput(&input);
				if (!IsZero(direction))
				{
					MoveEntity(map, entity, direction);
					AcceptTurn(turns);
				}
			}
			else
			{
				// NOTE(): Enemy behaviour goes here.
				// switch (entity->behaviour) ... etc.

				// TODO(): IMPORTANT! We should make our own rand() and stop using
				// the CRT one altogether. Just in case we'll ever need to have a reliable determinism.
				v2s directions[4] = { Up(), Down(), Left(), Right() };
				s32 direction = rand() % ArraySize(directions);
				MoveEntity(map, entity, directions[direction]);
				AcceptTurn(turns);

				// TODO(): We should either have like a few seconds of delay here,
				// during which an animation plays out,
				// OR exhaust all of the remaining turns in the queue in this single frame.
				// Also, the system propably should support doing few entity moves in
				// a single turn, and smoothly animating each one of those steps.
			}
		}
	}
}

fn void DrawFrame(game_world_t *state, command_buffer_t *out, f32 dt)
{
	DrawRect(out, V2(0, 0), V2(1920, 1080), DarkBlue());

	// NOTE(): The camera offset should be passed to the renderer as
	// some kind of "set transform" command maybe.
	const map_t *map = state->map;
	for (s32 y = 0; y < map->y; y++)
	{
		for (s32 x = 0; x < map->x; x++)
			RenderIsoTile(out, map, V2s(x, y), state->camera_offset, White());
	}

	entity_storage_t *storage = state->storage;
	for (s32 index = 0; index < storage->num; index++)
	{
		entity_t *entity = &storage->entities[index];
		DebugPrint("entity[%i] %i, %i", index, entity->p.x, entity->p.y);

		v2 entity_half_sz = Scale(state->entity_size, 0.5f);
		v2 target = GetTileCenter(state->map, entity->p);
		target = Sub(target, entity_half_sz);
		entity->deferred_p = Lerp2(entity->deferred_p, target, 10.0f * dt);

		v2 p = ScreenToIso(entity->deferred_p);
		p = Add(p, state->camera_offset);

		v4 color = (entity->flags & entity_flags_controllable) ? Pink() : Red();
		RenderIsoCube(out, p, state->entity_size, ENTITY_PIXEL_HEIGHT, color);
	}
}