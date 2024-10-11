typedef u64 entity_id_t;

typedef enum
{
	entity_flags_controllable = 1 << 0,
	entity_flags_hostile = 1 << 1,
} entity_flags_t;

typedef enum {
	ITEM_TYPE_WEAPON,
	ITEM_TYPE_CONSUMABLE,
	ITEM_TYPE_ARMOR,
	ITEM_TYPE_MISC,
} item_type_t;

typedef struct {
	u32 id;
	char name[32];
	item_type_t type;
	u16 damage, damage_threshold, healing_amount;
	u16 value, weight;
} item_t;

#include "settings.h"
typedef struct
{
	u8 flags;
	entity_id_t id;
	v2s p; // A position on the tile map.
	v2 deferred_p;

	u16 health, max_health;
	u16 max_carry_weight, carried_weight;
	u16 attack_dmg;
	u16 attack_turns;
	item_t* inventory[MAX_INVENTORY_SIZE];
	item_t* equipped_weapon;
	item_t* equipped_armor;
	u8 inventory_count;

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
	f32 time;

	s32 action_points;
	s32 turn_inited;
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
#include "turn_based.c"

fn void Setup(game_world_t *state, memory_t *memory, log_t *log)
{
	state->cursor = PushStruct(cursor_t, memory);
	state->turns = PushStruct(turn_queue_t, memory);
	state->storage = PushStruct(entity_storage_t, memory);
	state->map = CreateMap(30, 20, memory, TILE_PIXEL_SIZE);

	// NOTE(): We run out of the memory!
	Assert(state->cursor && state->turns && state->storage && state->map);

	u16 temp_player_health = 100;
	u16 temp_player_max_health = 100;
	u16 temp_attack_dmg = 40;
	u16 temp_max_carry_weight = 40;
	CreateEntity(state->storage, V2S(10, 5), entity_flags_controllable,
		temp_player_health, temp_attack_dmg, temp_max_carry_weight, temp_player_max_health);
	CreateEntity(state->storage, V2S(11, 5), entity_flags_controllable,
		temp_player_health, temp_attack_dmg, temp_max_carry_weight, temp_player_max_health);
	//CreateEntity(state->storage, V2S(12, 5), entity_flags_controllable);
	state->camera_position = V2(0, 0);
	
	LogLn(log, "Setup(): static memory budget is %i/%i MB", memory->offset / 1024 / 1024, memory->size / 1024 / 1024);
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
		
		// NOTE():  Maybe a new turn should be scheduled in *immediately* after the
		// current one ends?
	}
	if (entity)
	{
		if ((turns->turn_inited == false))
		{
			turns->action_points = (BeginTurn(state, entity) + 1);
			turns->turn_inited = true;
			#if ENABLE_TURN_SYSTEM_DEBUG_LOGS
			LogLn(log, "TurnKernel(): initiating turn for entity#%i", entity->id);
			#endif
			// NOTE(): Begin the turn.
		}

		Assert(turns->turn_inited);
		// NOTE(): The turn will "stall" until AcceptTurn() is called.

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
			// NOTE(): Enemy behaviour goes here.
			// switch (entity->behaviour) ... etc.

			f32 speed_mul = TURN_SPEED_NORMAL;
			if (IsKeyPressed(input, key_code_space))
				speed_mul = TURN_SPEED_FAST;

			turns->time += dt * speed_mul;
			if (turns->time >= 1.0f)
			{
				// NOTE(): Move the entity in a random direction.
				turns->time = 0.0f;
				if (turns->action_points > 0) // NOTE(): We can still make moves.
				{
					s32 cost = Decide(state, entity);
						turns->action_points -= cost;
				}
			}
			if (turns->action_points<=0)
				AcceptTurn(turns);
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
		entity->deferred_p = Lerp2(entity->deferred_p, GetTileCenter(state->map, entity->p), 10.0f * dt);

		v2 p = ScreenToIso(entity->deferred_p);
		v4 color = Red();
		v2 debug_alignment = V2(0.5f, 0.70f);
		bitmap_t *bitmap = &assets->Player[0];
		if (entity->flags & entity_flags_controllable)
		{
			color = Pink();
			//bitmap = &assets->Player[0];
			//debug_alignment = V2(0.5f, 0.9f);
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
// INVENTORY MANAGEMENT todo: create item instances, render inventory ui, implement item pickups in game world, render items in game world, update hud for quick slots
fn b32 AddItemToInventory(entity_t *entity, item_t *item) {
	if (entity->inventory_count < MAX_INVENTORY_SIZE) {
		u16 new_carried_weight = entity->carried_weight + item->weight;
		if (new_carried_weight <= entity->max_carry_weight) {
			entity->inventory[entity->inventory_count++] = item;
			return true;
		} else return false;
	} return false;
}

fn b32 RemoveItemFromInventory(entity_t *entity, u32 item_id) {
	for (u8 i = 0; i < entity->inventory_count; i++) {
		item_t *item = entity->inventory[i];
		if (item != NULL && entity->inventory[i]->id == item_id) {
			entity->carried_weight -= item->weight;

			for (u8 j = i; j < entity->inventory_count - 1; j++) {
				entity->inventory[j] = entity->inventory[j + 1];
			}
			entity->inventory[entity->inventory_count - 1] = NULL;
			entity->inventory_count--;
			return true;
		}
	}
	return false;
}

// equip, drink, eat, wear, etc.
fn void UseItem(entity_t *entity, u32 item_id) {
	for (u32 i = 0; i < entity->inventory_count; i++) {
		item_t *item = entity->inventory[i];
		if (item != NULL && item->id == item_id) {
			switch (item->type) {
				case ITEM_TYPE_CONSUMABLE:
					entity->health += item->healing_amount;
				if (entity->health > entity->max_health) {
					entity->health = entity->max_health;
				}
				RemoveItemFromInventory(entity, item->id);
					break;
				case ITEM_TYPE_WEAPON:
					if (entity->equipped_weapon)
						AddItemToInventory(entity, entity->equipped_weapon);
					entity->equipped_weapon = item;
					entity->attack_dmg = item->damage;
					break;
				case ITEM_TYPE_ARMOR:
					if (entity->equipped_armor)
						AddItemToInventory(entity, entity->equipped_armor);
					entity->equipped_armor = item;
					entity->attack_dmg = item->damage;
					break;
				case ITEM_TYPE_MISC:
					break;
				default:
					break;
			}
		}
	}
}

//-----------------------