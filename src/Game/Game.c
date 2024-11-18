// NOTE(): Map/
#include "Map/map.h"
#include "Map/map.c"
#include "Map/generator.h"
#include "Map/generator.c"
#include "Map/pathfinding.h"
#include "Map/pathfinding.c"

// NOTE(): Game/
#include "Game/Action.h"
#include "Game/items.h"
#include "Game/inventory.h"
#include "Game/inventory.c"

#include "Game/entity.h"
#include "Game/entity.c"
#include "Game/Action.c"

#include "Game/TurnSystem.h"
#include "Game/cursor.h"
#include "Game/particle.c"

#include "UI/interface.h"

typedef struct
{
	entity_id_t			Player;
	assets_t 			*assets;
	log_t 				*log;
	cursor_t 			*cursor;
	entity_storage_t 	*storage;
	particles_t 		*particles;
	interface_t 		*interface;
	slot_bar_t 			slot_bar;
	turn_system_t 		*turns;
	map_t 				*map;
	camera_t 			*camera;
	memory_t 			*memory;
	map_layout_t 		*layout;    
} game_state_t;

#include "Game/Scene.c"
#include "Renderer/Draw.c"

#include "Game/cursor.c"
#include "Game/enemy.c"
#include "Game/Player.c"
#include "Game/Camera.c"
#include "Game/TurnSystem.c"

#include "Renderer/Render.c"

#include "UI/hud_inventory.c"
#include "UI/hud_minimap.c"
#include "UI/hud_bar.c"
#include "UI/hud_queue.c"
#include "UI/hud.c"

fn void Setup(game_state_t *state, memory_t *memory, log_t *log, assets_t *assets)
{
	state->memory = memory;
	state->assets = assets;
	state->log = log;
	state->camera 		= PushStruct(camera_t, memory);
	state->cursor 		= PushStruct(cursor_t, memory);
	state->turns  		= PushStruct(turn_system_t, memory);
	state->storage 		= PushStruct(entity_storage_t, memory);
	state->particles 	= PushStruct(particles_t, memory);
	state->interface 	= PushStruct(interface_t, memory);
	state->layout       = PushStruct(map_layout_t, memory);
	state->map = AllocateMap(1024, 1024, memory, TILE_PIXEL_SIZE);
	
	SetupItemDataTable(memory, assets);
	SetupActionDataTable(memory, assets);
	SetupCamera(state->camera, V2(1600.0f, 900.0f));

	DefaultActionBar(&state->slot_bar,  assets);

	GenerateDungeon(state->layout, 6, *state->memory);
	CreateScene(state, state->layout);
}

fn void EstablishTurnOrder(turn_system_t *System)
{
	entity_storage_t *Storage = System->storage;
	PushTurn(System, GetEntity(System->storage, System->Player));
	
	for (s32 index = 0; index < Storage->EntityCount; index++)
	{
		entity_t *entity = &Storage->entities[index];
		if (IsHostile(entity) && entity->Alerted)
			PushTurn(System, entity);
	}
}

fn void TurnSystem(game_state_t *state, entity_storage_t *storage, map_t *map, turn_system_t *queue, f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out, assets_t *assets)
{
	entity_t *ActiveEntity = NULL;

	BeginTurnSystem(queue, state, dt);
	ActiveEntity = NextInOrder(queue, storage);

	b32 TurnHasEnded = (ActiveEntity == NULL);
	if (TurnHasEnded)
	{
		ClearTurnQueue(queue);
		EstablishTurnOrder(queue);
	}

	if (ActiveEntity)
	{
		if ((queue->turn_inited == false))
		{
			CloseCursor(state->cursor);
			CloseInventory(state->interface);
			
			s32 MovementPointCount = BeginTurn(state, ActiveEntity);
			SetupTurn(queue, MovementPointCount);
			
			Assert(queue->turn_inited);
		}

		Camera(state, ActiveEntity, input, dt);

		if (ActiveEntity->flags & entity_flags_controllable)
		{
			s32 Interupted = CheckTurnInterupts(state, ActiveEntity);
			if (!Interupted)
				Player(ActiveEntity, state, input, out, &cons);
		}
		else
		{
			AI(state, storage, map, queue, dt, input, cons, log, out, assets, ActiveEntity);
		}
	}

	ResolveAsynchronousActionQueue(queue, ActiveEntity, out, dt, assets, state);
	GarbageCollect(state, queue, dt);
	ControlPanel(state->turns, &cons, state->storage);

	EndTurnSystem(queue, state);
}

fn void Tick(game_state_t *state, f32 dt, client_input_t input, virtual_controls_t cons, command_buffer_t *Layer0, command_buffer_t *Layer1)
{
	TurnSystem(state, state->storage, state->map, state->turns, dt, &input, cons, state->log, Layer1, state->assets);	
	HUD(Debug.out, state, state->turns, state->storage, state->assets, &input, &cons, dt);
	Render_DrawFrame(state, Layer0, dt, state->assets, V2(input.viewport[0], input.viewport[1]));
}