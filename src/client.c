// NOTE(): Low/
#include "Game/settings.h"
#include "low/common.h"
#include "low/common.c"
#include "low/memory.h"
#include "low/vec.h"
#include "low/vec.c"
#include "low/colors.h"
#include "Renderer/bitmap.h"
#include "Renderer/bmfont.h"
#include "Renderer/bmfont.c"
#include "Renderer/renderer.h"
#include "Renderer/renderer.c"
#include "Renderer/assets.h"
#include "low/log.h"
#include "low/log.c"
#include "low/debug.h"
#include "low/debug.c"
#include "low/input.h"

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

#include "UI/hud.h"
#include "Game/Game.h"

// NOTE(): Editor/
#include "UI/editor.h"
#include "UI/editor.c"

typedef enum
{
	output_low,
	output_mid,
	output_high,
	output_count
} output_layer_t;

typedef struct
{
	virtual_controls_t virtual_controls;
	assets_t assets;
	bmfont_t font;
	editor_state_t editor;
	game_state_t world;
	command_buffer_t buffers[output_count];
	log_t *event_log;
	memory_t memory;
	u8 reserved[MB(256)];
	u8 keys_prev[256];
	b32 inited;
	f64 timestamp;
} 
client_t;

fn s32 Startup(client_t *state)
{
	memset(state, 0, sizeof(*state));

	memory_t *memory = &state->memory;
	memory->size = ArraySize(state->reserved);
	memory->_memory = state->reserved;

	command_buffer_t *buffers = state->buffers;
	buffers[0] = PushCommandBuffer(memory, 1024 * 1024);
	buffers[1] = PushCommandBuffer(memory, 1024 * 1024);
	buffers[2] = PushCommandBuffer(memory, 1024 * 1024);

	state->event_log = PushStruct(log_t, memory);
	ZeroStruct(state->event_log);
	
	LoadAssets(&state->assets, &state->font);

	s32 FontLoaded = LoadBMFont(&state->font, "assets/inconsolata.fnt");
	Assert(FontLoaded);

	return (TRUE);
}

fn void BeginFrame(client_t *state)
{
	for (s32 index = 0; index < ArraySize(state->buffers); index++)
		FlushCommandBuffer(&state->buffers[index]);
	BeginDebugFrame(&state->buffers[1], &state->buffers[1], &state->font, state->event_log);
}

fn void EndFrame(client_t *state, const client_input_t *input)
{
	for (s32 index = 0; index < 256; index++)
		state->keys_prev[index] = input->keys[index];
	EndDebugFrame();
}

fn s32 Host(client_t *state, render_output_t *output, client_input_t input)
{
	f32 dt = 1.0f / 60.0f;
	if (state->timestamp > 0.0f)
		dt = (f32)(input.time - state->timestamp);
	state->timestamp = input.time;

	BeginFrame(state);
	if (state->inited == 0)
	{
		Setup(&state->world, &state->memory, state->event_log, &state->assets);
		state->inited = TRUE;
	}

	virtual_controls_t virtual_cons = MapKeyboardToVirtualCons(&input, state->keys_prev);
	Editor(&state->editor, &state->world, &state->buffers[output_high], &input, state->event_log, &state->assets, &virtual_cons);
	Tick(&state->world, dt, input, virtual_cons, &state->buffers[output_low], &state->buffers[output_high]);	
		
	EndFrame(state, &input);

	MessageLog(&state->buffers[1], &state->font, V2(10.0f, 650.0f), state->event_log, dt);
	memset(output, 0, sizeof(*output));

	camera_t camera = *state->world.camera;
	PushRenderOutput(output, state->buffers[output_low], V4(0, 0, 1600, 900), camera);
	PushRenderOutput(output, state->buffers[output_high], V4(0, 0, 1600, 900), camera);
	PushRenderOutput(output, state->buffers[output_mid], V4(0, 0, 1600, 900), DefaultCamera()); // NOTE(): Debug.
	
	return (0);
}