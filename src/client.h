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
#include "debug.h"
#include "debug.c"
#include "input.h"
#include "map.h"
#include "map.c"
#include "dijkstra.c"
#include "world.h"

typedef struct
{
	bmfont_t font;
	game_world_t world;
	command_buffer_t buffers[2];
	memory_t memory;
	u8 reserved[MB(1)];
	f64 timestamp;
} client_t;

fn s32 Startup(client_t *state)
{
	memset(state, 0, sizeof(*state));

	memory_t *memory = &state->memory;
	memory->size = ArraySize(state->reserved);
	memory->memory = state->reserved;

	command_buffer_t *buffers = state->buffers;
	buffers[0] = PushCommandBuffer(memory, 1024 * 16);
	buffers[1] = PushCommandBuffer(memory, 1024);
	Setup(&state->world, &state->memory);

	s32 FontLoaded = LoadBMFont(&state->font, "assets/inconsolata.fnt");
	Assert(FontLoaded);

	return (TRUE);
}

fn void BeginFrame(client_t *state)
{
	for (s32 index = 0; index < ArraySize(state->buffers); index++)
		FlushCommandBuffer(&state->buffers[index]);
	BeginDebugFrame(&state->buffers[1], &state->font);
}

fn void EndFrame(client_t *state)
{
	EndDebugFrame();
}

fn s32 Host(client_t *state, render_output_t *output, client_input_t input)
{
	f32 dt = 1.0f / 60.0f;
	if (state->timestamp > 0.0f)
		dt = (f32)(input.time - state->timestamp);
	state->timestamp = input.time;

	BeginFrame(state);
	Update(&state->world, dt, input);
	DrawFrame(&state->world, &state->buffers[0], dt);
	EndFrame(state);

	memset(output, 0, sizeof(*output));
	PushRenderOutput(output, state->buffers[0]);
	PushRenderOutput(output, state->buffers[1]); // NOTE(): Debug.
	return (0);
}