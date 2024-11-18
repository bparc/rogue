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
	turn_queue_t 		*turns;
	map_t 				*map;
	camera_t 			*camera;
	memory_t 			*memory;
	map_layout_t 		*layout;

    container_t         containers[64];
    s32                 container_count;
} game_world_t;

#include "Game/Scene.h"
#include "Game/Scene.c"
#include "Game/Game.c"

#include "Renderer/draw.h"
#include "Renderer/draw.c"

#include "Game/cursor.c"
#include "Game/enemy.c"
#include "Game/Player.c"
#include "Game/TurnSystem.c"

#include "Renderer/Render.c"

#include "UI/hud_inventory.c"
#include "UI/hud_minimap.c"
#include "UI/hud_bar.c"
#include "UI/hud_queue.c"
#include "UI/hud.c"

fn void Setup(game_world_t *state, memory_t *memory, log_t *log, assets_t *assets)
{
	state->memory = memory;
	state->assets = assets;
	state->log = log;
	state->camera 		= PushStruct(camera_t, memory);
	state->cursor 		= PushStruct(cursor_t, memory);
	state->turns  		= PushStruct(turn_queue_t, memory);
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

fn void BeginGameWorld(game_world_t *state)
{

}

fn void EndGameWorld(game_world_t *state)
{

}

fn void Tick(game_world_t *state, f32 dt, client_input_t input, virtual_controls_t cons, command_buffer_t *Layer0, command_buffer_t *Layer1)
{
	BeginGameWorld(state);
	TurnSystem(state, state->storage, state->map, state->turns, dt, &input, cons, state->log, Layer1, state->assets);	
	EndGameWorld(state);

	HUD(Debug.out, state, state->turns, state->storage, state->assets, &input, &cons, dt);
	Render_DrawFrame(state, Layer0, dt, state->assets, V2(input.viewport[0], input.viewport[1]));
}