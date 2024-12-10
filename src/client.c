// NOTE(): Low/
#include "settings.h"
#include "low/common.h"
#include "low/common.c"
#include "low/memory.h"
#include "low/vec.h"
#include "low/vec.c"
#include "low/colors.h"
#include "renderer/bitmap.h"
#include "renderer/bmfont.h"
#include "renderer/bmfont.c"
#include "renderer/renderer.h"
#include "renderer/renderer.c"
#include "renderer/assets.h"
#include "low/log.h"
#include "low/log.c"
#include "low/debug.h"
#include "low/debug.c"
#include "low/input.h"

#include "game.c"

typedef enum
{
	output_low,
	output_mid,
	output_high,
	output_count
} output_layer_t;

typedef enum
{
	game_mode_menu,
	game_mode_game,
} game_mode_t;

typedef struct
{
	virtual_controls_t virtual_controls;
	assets_t assets;
	bmfont_t font;
	game_state_t world;
	command_buffer_t buffers[output_count];
	log_t *event_log;
	memory_t memory;
	u8 reserved[MB(256)];
	u8 keys_prev[256];
	b32 inited;
	f64 timestamp;

	f64 TotalTime;
	f32 ModeTime;
	memory_t GameMemory;
	game_mode_t Mode;
	menu_t Menu;
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

	state->GameMemory = Split(&state->memory, MB(64));

	return (TRUE);
}

fn void BeginFrame(client_t *state)
{
	for (s32 index = 0; index < ArraySize(state->buffers); index++)
		FlushCommandBuffer(&state->buffers[index]);
	BeginDebugFrame(&state->buffers[1], &state->buffers[1], &state->font, state->event_log);
}

fn void EndFrame(client_t *state, const client_input_t *input, f32 dt)
{
	for (s32 index = 0; index < 256; index++)
		state->keys_prev[index] = input->keys[index];
	EndDebugFrame();
	state->ModeTime += dt;
}

fn void GameMode(client_t *state, render_output_t *output, client_input_t input, virtual_controls_t virtual_cons, f32 dt)
{
	Tick(&state->world, dt, input, virtual_cons, &state->buffers[output_low], &state->buffers[output_high]);		
}

fn s32 Host(client_t *state, render_output_t *output, client_input_t input)
{
	ZeroStruct(output);

	f32 dt = 0.0f;
	if (state->timestamp > 0.0f)
		dt = (f32)(input.time - state->timestamp);
	state->timestamp = input.time;

	virtual_controls_t virtual_cons = MapKeyboardToVirtualCons(&input, state->keys_prev);
	BeginFrame(state);

	camera_t Camera = DefaultCamera();

	switch (state->Mode)
	{
	case game_mode_game:
		{
			GameMode(state, output, input, virtual_cons, dt);

			if (state->ModeTime <= 1.0f)
				DrawRect(&state->buffers[output_mid], V2(0.0f, 0.0f), V2(1600.0f, 900.0f), W(Black(), 1.0f - state->ModeTime));
			if (input.keys[key_code_f12])	
			{
				state->Mode = game_mode_menu;
				state->ModeTime = 0.0f;
				ZeroStruct(&state->Menu);
			}

			Camera = *state->world.Camera;
		} break;
	case game_mode_menu:
		{
			b32 RunGame = MenuMode(&state->Menu, &state->buffers[output_mid], &state->assets, &virtual_cons, state->ModeTime);
			if (RunGame)
			{

				FlushMemory(&state->GameMemory);
				Setup(&state->world, &state->GameMemory, state->event_log, &state->assets);
				state->ModeTime = 0.0f;
				state->Mode = game_mode_game;
			}
		} break;
	}

	MessageLog(&state->buffers[1], &state->font, V2(10.0f, 650.0f), state->event_log, dt);
	PushRenderOutput(output, state->buffers[output_low], V4(0, 0, 1600, 900), Camera);
	PushRenderOutput(output, state->buffers[output_high], V4(0, 0, 1600, 900), Camera);
	PushRenderOutput(output, state->buffers[output_mid], V4(0, 0, 1600, 900), DefaultCamera()); // NOTE(): Debug.

	EndFrame(state, &input, dt);
	
	return (0);
}