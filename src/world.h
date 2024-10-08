typedef u64 entity_id_t;

typedef enum
{
	entity_flags_controllable = 1 << 0,
	entity_flags_hostile = 1 << 1,
} entity_flags_t;

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

// NOTE(): Stores the turns as an list of entity ids
// in a *reverse* order (the last turn in the queue will be executed first).
typedef struct
{
	s32 num;
	entity_id_t entities[64];
} turn_queue_t;

// NOTE(): Directions.
static const v2s CardinalDirections[4] = { {0, -1}, {+1, 0}, {0, +1}, {-1, 0} };
static const v2s DiagonalDirections[4] = { {-1, -1}, {1, -1}, {1, +1}, {-1, +1}};

#include "settings.h"
typedef struct
{
	entity_storage_t *storage;
	turn_queue_t *turns;

	map_t *map;
	f32 time_elapsed;

	v2 camera_position;
	s32 moves_remaining;
} game_world_t;

#include "world.c"

//fn void BeginGameWorld(game_world_t *state);
//fn void EndGameWorld(game_world_t *state);

//fn void Setup(game_world_t *world, memory_t *memory);
//fn void Update(game_world_t *state, f32 dt, client_input_t input);
//fn void DrawFrame(game_world_t *state, command_buffer_t *out, f32 dt, assets_t *assets);

fn void Setup(game_world_t *state, memory_t *memory)
{
	state->turns = PushStruct(turn_queue_t, memory);
	state->storage = PushStruct(entity_storage_t, memory);
	state->map = CreateMap(20, 20, memory, TILE_PIXEL_SIZE);
	CreateEntity(state->storage, V2s(10, 5), entity_flags_controllable);
	CreateEntity(state->storage, V2s(8, 6), entity_flags_hostile);
	CreateEntity(state->storage, V2s(5, 12), entity_flags_hostile);
	CreateEntity(state->storage, V2s(4, 2), entity_flags_hostile);

	for (s32 index = 0; index < 8; index++)
	{
		SetTileValueI(state->map, 0, index, 1);
		SetTileValueI(state->map, 1, index, 1);
		SetTileValueI(state->map, 2, index, 1);
		SetTileValueI(state->map, 3, index, 1);
	}

	state->camera_position = V2(0, 0);
}

fn void BeginGameWorld(game_world_t *state)
{
	DebugPrint("Player Controls: WASD; Hold shift for diagonal input.");
	DebugPrint("Moves: %i", state->moves_remaining);
	// TODO(): This will break DebugPrints!
	// Let's make a separate output for those.
	SetGlobalOffset(Debug.out, state->camera_position);
}

fn void EndGameWorld(game_world_t *state)
{
	SetGlobalOffset(Debug.out, V2(0.0f, 0.0f));
}

fn void Update(game_world_t *state, f32 dt, client_input_t input)
{
	BeginGameWorld(state);

	map_t *map = state->map;
	entity_storage_t *storage = state->storage;
	turn_queue_t *turns = state->turns;
	if (turns->num == 0)
	{
		// NOTE(): We run out of the turns, time to schedule new ones.
		DefaultTurnOrder(turns, storage);
		state->moves_remaining = 4;
		// NOTE():  Maybe a new turn should be scheduled in *immediately* after the
		// current one ends?
	}

	// NOTE(): Print out the turn order.
	#if 0
	#if _DEBUG
		for (s32 index = turns->num - 1; index >= 0; index--)
			DebugPrint("%i, ", turns->entities[index]);
	#endif
	#endif

	if (turns->num > 0)
	{
		// NOTE(): Process the current turn
		entity_t *entity = NextInOrder(turns, storage);
		if (entity)
		{
			// NOTE(): We propably want to render stuff like this from here even
			// when NOT in the debug mode.
			#if _DEBUG
			RenderIsoTile(Debug.out, map, entity->p, Red(), false, 0);
			#endif

			// NOTE(): The turn will "stall" until AcceptTurn() is called.
			if (entity->flags & entity_flags_controllable)
			{
				// NOTE(): Track the entity with a camera.
				v2 player_world_pos = GetTileCenter(state->map, entity->p);
				v2 player_iso_pos = ScreenToIso(player_world_pos);
			
				v2 screen_center = Scale(GetViewport(&input), 0.5f);
				v2 camera_offset = Sub(screen_center, player_iso_pos);
				state->camera_position = Lerp2(state->camera_position, camera_offset, 5.0f * dt);

				// NOTE(): Listen for the player input.
				const v2s *considered_dirs = CardinalDirections;
				
				if (IsKeyPressed(&input, key_code_shift)) // TODO(): Implement key_code_shift on the GLFW backend.
					considered_dirs = DiagonalDirections;

				#if _DEBUG
				for (s32 index = 0; index < 4; index++)
					RenderIsoTile(Debug.out, map, AddS(entity->p, considered_dirs[index]), Orange(), true, 0);
				#endif

				s32 direction = GetDirectionalInput(&input);
				//Valid input
				if ((direction >= 0) && (direction < 4))
				{
					//future position
					v2s peekPos = AddS(entity -> p, considered_dirs[direction]);
					//valid move pos
					if(!IsOutOfBounds(state, peekPos) && !IsWall(state, peekPos)){

						MoveEntity(map, entity, considered_dirs[direction]);

						state->moves_remaining--;
						if (state->moves_remaining == 0)
							AcceptTurn(turns);
					}
				}
			}
			else
			{
				// NOTE(): Enemy behaviour goes here.
				// switch (entity->behaviour) ... etc.

				// NOTE(): Move the entity in a random direction.
				#if 1
				// TODO(): IMPORTANT! We should make our own rand() and stop using
				// the CRT one altogether. Just in case we'll ever need to have a reliable determinism.
				v2s directions[4] = { Up(), Down(), Left(), Right() };
				s32 direction = rand() % ArraySize(directions);
				MoveEntity(map, entity, directions[direction]);
				#endif
				AcceptTurn(turns);

				// TODO(): We should either have like a few seconds of delay here,
				// during which an animation plays out,
				// OR exhaust all of the remaining turns in the queue in this single frame.
				// Also, the system propably should support doing few entity moves in
				// a single turn, and smoothly animating each one of those steps.
			}
		}
	}

	EndGameWorld(state);
}

fn void DrawFrame(game_world_t *state, command_buffer_t *out, f32 dt, assets_t *assets)
{
	const map_t *map = state->map;
	entity_storage_t *storage = state->storage;

	SetGlobalOffset(out, V2(0.0f, 0.0f));
	DrawRect(out, V2(0, 0), V2(1920, 1080), DarkBlue()); // NOTE(): Background

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
				if (IsWall(state, (v2s) {x, y}))
				{
					Filled = true;
					height = 80;
				}

				RenderIsoTile(out, map, V2s(x, y), color, Filled, height);
			}
		}
	}

	// NOTE(): Render entities.
	
	for (s32 index = 0; index < storage->num; index++)
	{
		entity_t *entity = &storage->entities[index];
		entity->deferred_p = Lerp2(entity->deferred_p, GetTileCenter(state->map, entity->p), 10.0f * dt);

		v2 p = ScreenToIso(entity->deferred_p);
		v4 color = Red();

		if (entity->flags & entity_flags_controllable)
		{
			color = Pink();
		}		
		else
		{
			bitmap_t *bitmap = &assets->Slime;
			v2 bitmap_sz = bitmap->scale;
			v2 bitmap_half_sz = Scale(bitmap_sz, 0.5f);
			v2 bitmap_aligment = bitmap_half_sz;
			bitmap_aligment.y += 5.0f;

			v2 bitmap_p = Sub(p, bitmap_aligment);
			DrawBitmap(out, bitmap_p, bitmap_sz, PureWhite(), bitmap);

			//DrawRectOutline(out, bitmap_p, bitmap_sz, Orange());
		}

		RenderIsoCubeCentered(out, p, V2(ENTITY_SIZE, ENTITY_SIZE), ENTITY_PIXEL_HEIGHT, color);
	}
}