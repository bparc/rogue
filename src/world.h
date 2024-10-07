typedef struct
{
	v2s p;
	v2 deferred_p;
} entity_t;

typedef struct
{
	s32 num;
	entity_t entities[1024];
} entity_storage_t;

#include "settings.h"
typedef struct
{
	v2 entity_size;
	entity_storage_t *storage;

	map_t *map;
	f32 time_elapsed;
	v2 camera_position;
} game_world_t;

#include "world.c"

fn void Setup(game_world_t *world, memory_t *memory);
fn void Update(game_world_t *state, f32 dt, client_input_t input);
fn void DrawFrame(game_world_t *state, command_buffer_t *out, f32 dt);

fn void Setup(game_world_t *state, memory_t *memory)
{
	state->storage = PushStruct(entity_storage_t, memory);
	state->map = CreateMap(20, 20, memory, TILE_PIXEL_SIZE);
	CreateEntity(state->storage, V2s(5, 5), 0);
	CreateEntity(state->storage, V2s(8, 4), 0);
	CreateEntity(state->storage, V2s(2, 12), 0);
	CreateEntity(state->storage, V2s(4, 2), 0);
	state->entity_size = V2(ENTITY_SIZE, ENTITY_SIZE);

	state->camera_position = V2(0, 0);
}

fn void Update(game_world_t *state, f32 dt, client_input_t input)
{
	DebugPrint("%s %.0fx%.0f", input.gpu_driver_desc, input.viewport[0], input.viewport[1]);

	while (input.char_count--)
	{
		char key = (char)input.char_queue[input.char_count];

		v2s delta = {0};
		if (ToUpper(key) == 'D')
			delta.x++;
		if (ToUpper(key) == 'A')
			delta.x--;
		if (ToUpper(key) == 'W')
			delta.y--;
		if (ToUpper(key) == 'S')
			delta.y++;

		if ((delta.x != 0) || (delta.y != 0))
		{
			entity_t *entity = &state->storage->entities[0];
			map_t *map = state->map;

			v2s new_p = AddS(entity->p, delta);
			new_p.x = ClampS32(new_p.x, 0, map->x - 1);
			new_p.y = ClampS32(new_p.y, 0, map->y - 1);
			entity->p = new_p;
		}
	}

	state->time_elapsed += dt;
}

fn void DrawFrame(game_world_t *state, command_buffer_t *out, f32 dt)
{
	DrawRect(out, V2(0, 0), V2(1920, 1080), DarkBlue());

	// NOTE(): The camera offset should be passed to the renderer as
	// some kind of "set transform" command maybe.
	//v2 camera_offset = V2(700.0f, 0.0f);

	entity_t* player_entity = &state->storage->entities[0];
	v2 player_world_pos = GetTileCenter(state->map, player_entity->p);

	v2 player_iso_pos = ScreenToIso(player_world_pos);

	v2 screen_center = V2(1600.0f/2, 900.0f/2);
	v2 camera_offset = Sub(screen_center, player_iso_pos);
	
	state->camera_position = Lerp2(state->camera_position, camera_offset, 5.0f * dt);

	const map_t *map = state->map;
	for (s32 y = 0; y < map->y; y++)
	for (s32 x = 0; x < map->x; x++)
	{
		v2 p = MapToScreen(map, V2s(x, y));
		p = ScreenToIso(p);
		p = Add(p, state->camera_position);
		RenderIsoCube(out, p, map->tile_sz, 0, White());
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

		v2 p = entity->deferred_p;
		p = ScreenToIso(p);

		p = Add(p, state->camera_position);

		v4 color = index > 0 ? Red() : Pink();
		RenderIsoCube(out, p, state->entity_size, ENTITY_PIXEL_HEIGHT, color);
	}
}