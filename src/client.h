#include "settings.h"
#include "common.h"
#include "common.c"
#include "memory.h"
#include "vec.h"
#include "vec.c"
#include "colors.h"
#include "bitmap.h"
#include "bmfont.h"
#include "bmfont.c"
#include "renderer.h"
#include "renderer.c"
#include "log.h"
#include "log.c"
#include "debug.h"
#include "debug.c"
#include "input.h"
#include "assets.h"
#include "map.h"
#include "map.c"
#include "pathfinding.h"
#include "pathfinding.c"
#include "world.h"
#include "generator.h"
#include "generator.c"
#include "editor.h"
#include "editor.c"

typedef struct
{
	virtual_controls_t virtual_controls;
	assets_t assets;
	bmfont_t font;
	editor_state_t editor;
	game_world_t world;
	command_buffer_t buffers[3];
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
	buffers[2] = PushCommandBuffer(memory, 1024 * 1024);
	buffers[1] = PushCommandBuffer(memory, 1024 * 1024);

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
	Editor(&state->editor, &state->world, &state->buffers[2], &input, state->event_log, &state->assets, &virtual_cons);

	Update(&state->world, dt, input, state->event_log, &state->assets, virtual_cons, &state->buffers[2]);
	DrawFrame(&state->world, &state->buffers[0], dt, &state->assets, V2(input.viewport[0], input.viewport[1]));
	
	EndFrame(state, &input);

	MessageLog(&state->buffers[1], &state->font, V2(10.0f, 650.0f), state->event_log, dt);
	memset(output, 0, sizeof(*output));

	camera_t camera = *state->world.camera;
	PushRenderOutput(output, state->buffers[0], V4(0, 0, 1600, 900), camera);
	PushRenderOutput(output, state->buffers[2], V4(0, 0, 1600, 900), camera);
	PushRenderOutput(output, state->buffers[1], V4(0, 0, 1600, 900), DefaultCamera()); // NOTE(): Debug.
	
	return (0);
}