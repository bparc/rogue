
typedef enum
{
	key_code_alt,
	key_code_shift,	
	key_code_space,
	key_code_tab,
	key_code_0,
	key_code_1,
	key_code_2,
	key_code_3,
	key_code_4,
	key_code_5,
	key_code_6,
	key_code_7,
	key_code_8,
	key_code_9,
	key_code_f1,
	key_code_f2,
	key_code_f3,
	key_code_f4,
	key_code_f5,
	// NOTE(): Key codes for letter keys
	// are the same as the coresponding
	// ASCII characters.
} key_code_t;

typedef struct
{
	f32 viewport[2];
	f32 mouse[2];
	s32 wheel;
	s32 mouse_buttons[2];
	s32 char_count;
	u8 char_queue[16];
	u8 keys[256];
	f64 time;
	const char *gpu_driver_desc;
} client_input_t;

fn inline v2 GetViewport(const client_input_t *input)
{
	return V2(input->viewport[0], input->viewport[1]);
}

fn inline v2 GetCursorOffset(const client_input_t *input)
{
	v2 result = V2((f32)input->mouse[0], (f32)input->mouse[1]);
	return result;
}

fn b32 IsKeyPressed(const client_input_t *input, u8 key)
{
	return input->keys[key];
}

typedef enum
{
	button_transition_none,
	button_transition_up,
	button_transition_down,
} button_transition_t;

typedef struct
{
	b32 state;
	button_transition_t transition;
} pad_button_t;

typedef struct
{
	pad_button_t dpad_up;
	pad_button_t dpad_right;
	pad_button_t dpad_down;
	pad_button_t dpad_left;
	pad_button_t Modal;

	pad_button_t confirm, cancel;

	union { pad_button_t x, SnapCursor; };
	union { pad_button_t y, EndTurn; };

	pad_button_t Inventory, select;
	pad_button_t rb;
	pad_button_t rotate;

	pad_button_t debug[12];
} virtual_controls_t;

fn inline b32 WentUp(pad_button_t button)
{
	return (button.transition == button_transition_up);
}

fn inline b32 WentDown(pad_button_t button)
{
	return (button.transition == button_transition_down);
}

fn inline b32 IsDown(pad_button_t button)
{
	return (button.state);
}

fn pad_button_t MapVirtualButton(key_code_t code, const client_input_t *input, u8 keys_prev[256])
{
	pad_button_t result = {0};
	result.state = input->keys[code];
	if(!input->keys[code] && keys_prev[code])
		result.transition = button_transition_up;
	if(input->keys[code] && !keys_prev[code])
		result.transition = button_transition_down;
	return result;
}

fn virtual_controls_t MapKeyboardToVirtualCons(const client_input_t *input, u8 keys_prev[256])
{
	virtual_controls_t result = {0};
	result.dpad_up  	= MapVirtualButton('W', input, keys_prev);
	result.dpad_right 	= MapVirtualButton('D', input, keys_prev);
	result.dpad_down 	= MapVirtualButton('S', input, keys_prev);
	result.dpad_left 	= MapVirtualButton('A', input, keys_prev);
	result.confirm		= MapVirtualButton('E', input, keys_prev);
	result.cancel		= MapVirtualButton('Q', input, keys_prev);
	result.Inventory	= MapVirtualButton(key_code_tab, input, keys_prev);
	result.rotate		= MapVirtualButton('R', input, keys_prev);
	result.debug[0]		= MapVirtualButton(key_code_f1, input, keys_prev);
	result.debug[1]		= MapVirtualButton(key_code_f2, input, keys_prev);
	result.debug[2]		= MapVirtualButton(key_code_f3, input, keys_prev);
	result.debug[3]		= MapVirtualButton(key_code_f4, input, keys_prev);
	result.debug[4]		= MapVirtualButton(key_code_f5, input, keys_prev);
	result.x			= MapVirtualButton(key_code_tab, input, keys_prev);
	result.y			= MapVirtualButton(key_code_space, input, keys_prev);
	result.select		= MapVirtualButton('B', input, keys_prev);
	result.Modal        = MapVirtualButton(key_code_shift, input, keys_prev);
	return result;
}

enum
{
	Dir_Invalid = -1,
};

// NOTE(): Directions
static const v2s cardinal_directions[4] = { {0, -1}, {+1, 0}, {0, +1}, {-1, 0} };
static const v2s diagonal_directions[4] = { {-1, -1}, {1, -1}, {1, +1}, {-1, +1}};

fn s32 IsDirValid(s32 Dir)
{
	s32 result = (Dir >= 0 && Dir < 4);
	return result;
}

typedef struct
{
	v2s Dirs[4];
	s32 DirIndex;
	v2s Direction;
	s32 Inputed;
} dir_input_t;

fn dir_input_t GetDirectionalInput(const virtual_controls_t *In)
{
	dir_input_t Result = {0};
	Result.DirIndex = -1;

	const v2s *Dirs = cardinal_directions;
	if (IsDown(In->Modal))
		Dirs = diagonal_directions;
	Result.Dirs[0] = Dirs[0];
	Result.Dirs[1] = Dirs[1];
	Result.Dirs[2] = Dirs[2];
	Result.Dirs[3] = Dirs[3];

	if (WentDown(In->dpad_up))
		Result.DirIndex = 0;
	else
	if (WentDown(In->dpad_right))
		Result.DirIndex = 1;
	else
	if (WentDown(In->dpad_down))
		Result.DirIndex = 2;
	else
	if (WentDown(In->dpad_left))
		Result.DirIndex = 3;
	
	if (Result.DirIndex >= 0)
	{
		Result.Inputed = true;
		Result.Direction = Dirs[Result.DirIndex];
	}
	return Result;
}