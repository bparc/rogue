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

#include "game/cursor.h"
#include "interface/interface.h"

#include "game.h"
#include "menu.h"
#include "menu.c"
#include "data.c"
#include "game/dungeon.c"

#include "renderer/draw.c"

#include "game/cursor.c"
#include "game/enemy.c"
#include "game/player.c"
#include "game/camera.c"
#include "game.c"

#include "render.c"

#include "interface/inventory.c"
#include "interface/minimap.c"
#include "interface/bar.c"
#include "interface/queue.c"
#include "interface/hud.c"

fn void Setup(game_state_t *State, memory_t *Memory, log_t *Log, assets_t *Assets)
{
	ZeroStruct(State);
	
	State->Memory = Memory;
	State->Assets = Assets;
	State->Log = Log;
	
	SetupMap(&State->Map, 1024, 1024, Memory, TILE_PIXEL_SIZE);

	SetupItemDataTable(Memory, Assets);
	SetupActionDataTable(Memory, Assets);

	SetupActionBar(&State->Bar, Assets);

	SetupCamera(&State->Camera, V2(1600.0f, 900.0f));
	State->Camera.zoom = 3.0f;

	GenerateDungeon(&State->MapLayout, 8, *Memory);
	CreateDungeon(State, &State->MapLayout);

	DebugLog("used memory %i/%i KB", Memory->offset / 1024, Memory->size / 1024);
}

fn inline void UpdateCurrentUnit(game_state_t *State, entity_t *Entity, f32 dt, client_input_t *input, virtual_controls_t cons, command_buffer_t *out)
{
	if ((State->TurnInited == false))
	{
		CloseCursor(&State->Cursor);
		CloseInventory(&State->GUI);			

		if (IsHostile(Entity))
			BeginEnemyTurn(State, Entity, *State->Memory);

		SetupTurn(State, 16);
		ProcessStatusEffects(Entity);

		IntegrateRange(&State->EffectiveRange, &State->Map, Entity->p, *State->Memory);
	}

	Camera(State, Entity, input, dt);

	if (Entity->flags & entity_flags_controllable)
	{
		if (!CheckEnemyAlertStates(State, Entity))
		{
			b32 BlockMovementInputs = IsCursorEnabled(&State->Cursor) ? true : false;
			dir_input_t DirInput = GetDirectionalInput(&cons);
			DoCursor(State, &State->Camera, &State->Cursor, State->Assets, &State->Bar, out, cons, Entity, DirInput);
			Player(Entity, &State->GUI, State, input, out, &cons, DirInput, BlockMovementInputs);
		}
	}
	else
	{
		AI(State, out, Entity);
	}
}

fn void Tick(game_state_t *State, f32 dt, client_input_t input, virtual_controls_t cons, command_buffer_t *Layer0, command_buffer_t *Layer1)
{
	BeginTurnSystem(State, State, dt);
	entity_t *Entity = GetActive(State);

	if (Entity == NULL)
	{
		ClearTurnQueue(State);
		EstablishTurnOrder(State);
	}

	if (Entity)
	{
		UpdateCurrentUnit(State, Entity, dt, &input, cons, Layer1);
	}

	UpdateAsynchronousActionQueue(State, Entity, dt, Layer1);
	
	GarbageCollect(State, State, dt);

	if ((State->Events & system_any_evicted))
	{
		CheckEncounterModeStatus(State);
	}
	EndTurnSystem(State, State);

	ControlPanel(State, &cons);

	HUD(Debug.out, State, &input, &cons, dt);
	Render_DrawFrame(State, Layer0, dt, V2(input.viewport[0], input.viewport[1]));
}