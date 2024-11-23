// NOTE(): Map/
#include "Levels.h"
#include "Map/map.h"
#include "Map/map.c"
#include "Map/generator.h"
#include "Map/generator.c"
#include "Map/pathfinding.h"
#include "Map/pathfinding.c"

// NOTE(): Game/
#include "Game/particle.c"

#include "Game/Action.h"
#include "Game/items.h"
#include "Game/inventory.h"
#include "Game/inventory.c"

#include "Game/entity.h"
#include "Game/entity.c"
#include "Game/Action.c"

#include "Game/TurnSystem.h"
#include "Game/cursor.h"

#include "UI/interface.h"
#include "GameMenu.c"

typedef struct
{
	entity_id_t			player;
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

#include "Game/Dungeon.c"
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

#include "GameData.h"

fn void Setup(game_state_t *state, memory_t *memory, log_t *log, assets_t *assets)
{
	DebugLog("");

	ZeroStruct(state);

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

	GenerateDungeon(state->layout, 12, *state->memory);
	CreateDungeon(state, state->layout);

	DebugLog("used memory %i/%i KB", memory->offset / 1024, memory->size / 1024);
}

fn void TurnSystem(game_state_t *state, entity_storage_t *storage, map_t *map, turn_system_t *queue, f32 dt, client_input_t *input, virtual_controls_t cons, log_t *log, command_buffer_t *out, assets_t *assets)
{
	entity_t *ActiveEntity = NULL;
	ControlPanel(state->turns, &cons, state->storage);

	BeginTurnSystem(queue, state, dt);
	ActiveEntity = PullUntilActive(queue);

	b32 TurnHasEnded = (ActiveEntity == NULL);
	if (TurnHasEnded)
	{
		ClearTurnQueue(queue);
		EstablishTurnOrder(queue);
		ActiveEntity = PullUntilActive(queue);
	}

	if (ActiveEntity)
	{
		if ((queue->turn_inited == false))
		{
			CloseCursor(state->cursor);
			CloseInventory(state->interface);			

			s32 MovementPointCount = MAX_PLAYER_MOVEMENT_POINTS;
			if (IsHostile(ActiveEntity))
				MovementPointCount = BeginEnemyTurn(queue, ActiveEntity, *state->memory);

			SetupTurn(queue, MovementPointCount, 4);
			ProcessStatusEffects(ActiveEntity);
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
			AI(state, dt, out, cons, ActiveEntity);
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
	TurnSystem(state, state->storage, state->map, state->turns, dt, &input, cons, state->log, Layer1, state->assets);	
	HUD(Debug.out, state, &input, &cons, dt);
	Render_DrawFrame(state, Layer0, dt, state->assets, V2(input.viewport[0], input.viewport[1]));
}