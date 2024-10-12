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

	u16 health;
	u16 attack_dmg;

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

#include "settings.h"
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
#include "turn_based.c"

fn void Setup(game_world_t *state, memory_t *memory)
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
}

fn void BeginGameWorld(game_world_t *state)
{
	DebugPrint("Player Controls: WASD; Hold shift for diagonal input.");
	DebugPrint("Cursor Controls: Press ALT to open the cursor. Press space to close it.");
	DebugPrint("Moves: %i", state->turns->action_points);
	// TODO(): This will break DebugPrints!
	// Let's make a separate output for those.
	SetGlobalOffset(Debug.out, state->camera_position);
}

fn void EndGameWorld(game_world_t *state)
{
	SetGlobalOffset(Debug.out, V2(0.0f, 0.0f));
}

fn void TurnKernel(game_world_t *state, entity_storage_t *storage, map_t *map, turn_queue_t *turns, f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out)
{
	SetGlobalOffset(out, state->camera_position); // NOTE(): Let's pass the camera position via the PushRenderOutput call instead of this SetGlobalOffset stuff.
	// NOTE(): Process the current turn
	entity_t *entity = NextInOrder(turns, storage);
	if (entity == 0)
	{
		// NOTE(): We run out of the turns, time to schedule new ones.
		EstablishTurnOrder(state, turns, storage);
	}
	if (entity)
	{
		if ((turns->turn_inited == false))
		{
			turns->action_points = (BeginTurn(state, entity) + 1);
			turns->turn_inited = true;

			turns->interp_state = interp_request;
			turns->time = 0.0f;
			turns->time_elapsed = 0.0f;

			#if ENABLE_TURN_SYSTEM_DEBUG_LOGS
			LogLn(log, "TurnKernel(): initiating turn for entity#%i", entity->id);
			#endif
			// NOTE(): Begin the turn.
		}

		Assert(turns->turn_inited);
		// NOTE(): The turn will "stall" until AcceptTurn() is called.

		// NOTE(): DEBUG draw the entity's"discrete" p.
		#if _DEBUG
		RenderIsoTile(out, map, entity->p, Red(), false, 0);
		#endif

		state->camera_position = CameraTracking(state->camera_position, entity->deferred_p, GetViewport(input), dt);
		
		if (entity->flags & entity_flags_controllable) // NOTE(): The "player" code.
		{
			// NOTE(): Listen for the player input.
			const v2s *considered_dirs = cardinal_directions;
			#if ENABLE_DIAGONAL_MOVEMENT
			if (IsKeyPressed(input, key_code_shift))
				considered_dirs = diagonal_directions;
			#endif
			
			s32 direction = GetDirectionalInput(input);
			b32 input_valid = (direction >= 0) && (direction < 4);
			b32 cursor_mode_active = state->cursor->active; // NOTE(): The cursor_active flag needs to be stored *before* calling DoCursor. This is actually the correct order. For reasons.

			DoCursor(out, entity, cons, input_valid,
				direction, considered_dirs, turns, map, storage, log, state->cursor);
			
			#if _DEBUG // NOTE(): Render the considered directions on the map.
			v2s base_p = cursor_mode_active ? state->cursor->p : entity->p;
			for (s32 index = 0; index < 4; index++)
				RenderIsoTile(out, map, AddS(base_p, considered_dirs[index]), SetAlpha(Orange(), 0.5f), true, 0);
			#endif
			
			//Valid input
			if (input_valid && (cursor_mode_active == false))
			{
				//future position
				v2s peekPos = AddS(entity -> p, considered_dirs[direction]);

				//valid move pos
				if(!IsOutOfBounds(state, peekPos) && !IsWall(state, peekPos))
				{
					if (turns->action_points > 0)
						MoveEntity(map, entity, considered_dirs[direction]);

					// NOTE(): Consume moves
					turns->action_points--;
					if (turns->action_points <= 0)
						AcceptTurn(turns);
				}
			}
		}
		else
		{
			// NOTE(): Animator
			f32 speed_mul = TURN_SPEED_NORMAL;
			turns->time += dt * speed_mul;
			turns->time_elapsed += dt;

			switch(turns->interp_state)
			{
			case interp_request:
				{
					// TODO(): Move the player code somewhere around here maybe?
					// It could potentially be a better way to structure this.
					turns->starting_p = entity->p;

					s32 cost = Decide(state, entity);
						turns->action_points -= cost;
					
					turns->interp_state = interp_transit;
					turns->time = 0.0f;
				} break;
			case interp_transit:
				{
					v2 a = GetTileCenter(map, turns->starting_p);
					v2 b = GetTileCenter(map, entity->p);
					entity->deferred_p = Lerp2(a, b, turns->time);
					if ((turns->time >= 1.0f))
					 {
					 	if (turns->action_points > 0)
					 	{
					 		turns->interp_state = interp_request;
					 	}
					 	else
					 	{
					 		s32 attempt = AttemptAttack(state, entity);
							turns->interp_state = attempt ? interp_attack : interp_accept;	
					 	}
						
						turns->time = 0.0f;
					}
				} break;
			case interp_attack:
				{
					DebugText(Add(ScreenToIso(entity->deferred_p), V2(120.0f, 60.0f)), "(I AM ATTACKING RIGHT NOW)");
					if ((turns->time >= 2.0f))
					{
						turns->interp_state = interp_accept;
						turns->time = 0.0f;
					}
				} break;
			case interp_accept:
				{
					if (turns->time > 0.1f)
					{
						AcceptTurn(turns);

						#ifdef ENABLE_TURN_SYSTEM_DEBUG_LOGS
						LogLn(log, "TurnKernel(): turn finished in %.2f seconds", turns->time_elapsed);
						#endif
					}
				} break;
			}
		}
	}
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