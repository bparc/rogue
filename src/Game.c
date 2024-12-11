// NOTE(): Map/
#include "Map/Map.h"
#include "Map/Map.c"
#include "Map/generator.h"
#include "Map/generator.c"
#include "Map/pathfinding.h"
#include "Map/pathfinding.c"

// NOTE(): Game/
#include "game/particle.c"

#include "game/action.h"
#include "game/items.h"
#include "game/inventory.h"


#include "game/entity.h"
#include "game/entity.c"
#include "game/action.c"
#include "game/inventory.c"

#include "game/turnbased.h"
#include "game/cursor.h"

#include "interface/interface.h"
#include "menu.c"

typedef struct
{
	entity_id_t			player;
	assets_t 			*assets;
	log_t 				*log;
	cursor_t 			*cursor;
	entity_storage_t 	*storage;
	particles_t 		*particles;
	interface_t 		*interface;
	slot_bar_t 			Bar;
	turn_system_t 		*System;
	map_t 				*Map;
	camera_t 			*Camera;
	memory_t 			*memory;
	map_layout_t 		*layout;   
} game_state_t;

#include "data.c"
#include "game/dungeon.c"
#include "renderer/draw.c"

#include "game/cursor.c"
#include "game/enemy.c"
#include "game/player.c"
#include "game/Camera.c"
#include "game/turnbased.c"

#include "render.c"

#include "interface/inventory.c"
#include "interface/minimap.c"
#include "interface/bar.c"
#include "interface/queue.c"
#include "interface/hud.c"

fn void Setup(game_state_t *state, memory_t *memory, log_t *log, assets_t *assets)
{
	DebugLog("");

	ZeroStruct(state);

	state->memory = memory;
	state->assets = assets;
	state->log = log;
	state->Camera 		= PushStruct(camera_t, memory);
	state->cursor 		= PushStruct(cursor_t, memory);
	state->System  		= PushStruct(turn_system_t, memory);
	state->storage 		= PushStruct(entity_storage_t, memory);
	state->particles 	= PushStruct(particles_t, memory);
	state->interface 	= PushStruct(interface_t, memory);
	state->layout       = PushStruct(map_layout_t, memory);
	state->Map = AllocateMap(1024, 1024, memory, TILE_PIXEL_SIZE);
	
	SetupItemDataTable(memory, assets);
	SetupActionDataTable(memory, assets);
	SetupCamera(state->Camera, V2(1600.0f, 900.0f));
	state->Camera->zoom = 3.0f;

	DefaultActionBar(&state->Bar, assets);

	GenerateDungeon(state->layout, 8, *state->memory);
	CreateDungeon(state, state->layout);

	DebugLog("used memory %i/%i KB", memory->offset / 1024, memory->size / 1024);
}

fn void TurnSystem(game_state_t *state, entity_storage_t *storage, map_t *Map, turn_system_t *queue, f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out, assets_t *assets)
{
	entity_t *ActiveEntity = NULL;
	ControlPanel(state->System, &cons, state->storage);

	BeginTurnSystem(queue, state, dt);
	ActiveEntity = Pull(queue);

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

			if (IsHostile(ActiveEntity))
				BeginEnemyTurn(queue, ActiveEntity, *state->memory);

			SetupTurn(queue, 16);
			ProcessStatusEffects(ActiveEntity);

			IntegrateRange(&queue->EffectiveRange, queue->Map, ActiveEntity->p, *state->memory);
		}

		Camera(state, ActiveEntity, input, dt);

		if (ActiveEntity->flags & entity_flags_controllable)
		{
			s32 Interupted = CheckEnemyAlertStates(state, ActiveEntity);
			if (!Interupted)
			{
				b32 BlockMovementInputs = false;
				if (IsCursorEnabled(state->cursor))
					BlockMovementInputs = true;

				dir_input_t DirInput = GetDirectionalInput(&cons);
				DoCursor(state, out, cons, ActiveEntity, DirInput);
				Player(ActiveEntity, state, input, out, &cons, DirInput, BlockMovementInputs);
			}
		}
		else
		{
			AI(queue, out, ActiveEntity);
		}
	}

	//
	ResolveAsynchronousActionQueue(queue, ActiveEntity, out, dt, assets, state);
	// ...
	GarbageCollect(state, queue, dt);

	if ((queue->Events & system_any_evicted))
		CheckEncounterModeStatus(queue);

	EndTurnSystem(queue, state);
}

fn void Tick(game_state_t *state, f32 dt, client_input_t input, virtual_controls_t cons, command_buffer_t *Layer0, command_buffer_t *Layer1)
{
	TurnSystem(state, state->storage, state->Map, state->System, dt, &input, cons, state->log, Layer1, state->assets);	
	HUD(Debug.out, state, &input, &cons, dt);
	Render_DrawFrame(state, Layer0, dt, state->assets, V2(input.viewport[0], input.viewport[1]));
}