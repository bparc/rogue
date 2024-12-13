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
#include "dungeon.c"

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

fn void Tick(game_state_t *State, f32 dt, client_input_t input, virtual_controls_t cons, command_buffer_t *Layer0, command_buffer_t *Layer1)
{
	entity_t *Entity = NULL;

	BeginTurnSystem(State, State, dt);
	Entity = GetActive(State);

	if (Entity == NULL)
	{
		EstablishTurnOrder(State);
	}

	if ((State->TurnInited == false) && Entity)
	{
		State->TurnInited = true;
		
		s32 AP = 16;
		SetupTurn(State, AP);
		
		IntegrateRange(&State->EffectiveRange, &State->Map, Entity->p, *State->Memory);
		EvaluateStatusEffects(Entity);

		CloseCursor(&State->Cursor);
		CloseInventory(&State->GUI);			
	}

	Camera(State, Entity, &input, dt);

	if (Entity && State->TurnInited)
	{
		b32 Update = true;

		if (Entity->flags & entity_flags_controllable)
		{
			// NOTE(): Skip the update for this frame if any of the enemies was alerted
			Update = CheckEnemyAlertStates(State, Entity) ? false : Update;
		}

		if (Update && (Entity->flags & entity_flags_controllable))
		{
			dir_input_t Dir = GetDirectionalInput(&cons);
			b32 BlockInputs = false;
			
			if (IsCursorEnabled(&State->Cursor))
				BlockInputs = true;

			UpdateAndRenderCursor(State, &State->Cursor, Layer1, cons, Entity, Dir);
			UpdatePlayer(State, Entity, &input, &cons, Dir, BlockInputs);
		}

		if (Update && !(Entity->flags & entity_flags_controllable))
		{
			UpdateAI(State, Entity);
		}

		if (State->EncounterModeEnabled)
		{
			RenderRangeMap(Layer1, &State->Map, &State->EffectiveRange);
		}
	}

	UpdateAsynchronousActionQueue(State, Entity, dt, Layer1);

	garbage_collect_result_t GarbageCollectResult = GarbageCollect(State, State, dt);
	if (GarbageCollectResult.DeletedEntityCount > 0)
	{
		CheckEncounterModeStatus(State);
	}

	ControlPanel(State, &cons);
	
	HUD(Debug.out, State, &input, &cons, dt);

	EndTurnSystem(State, State);

	Render_DrawFrame(State, Layer0, dt, V2(input.viewport[0], input.viewport[1]));
}