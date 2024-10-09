typedef enum
{
	key_code_alt,
	key_code_shift,	
	key_code_space,
	key_code_tab,
	// NOTE(): Key codes for letter keys
	// are the same as the coresponding
	// ASCII characters.
} key_code_t;

typedef struct
{
	f32 viewport[2];
	f32 mouse[2];
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

fn inline v2 GetCursorP(const client_input_t *input)
{
	v2 result = V2((f32)input->mouse[0], (f32)input->mouse[1]);
	return result;
}

fn s32 GetDirectionalInput(const client_input_t *input)
{
	s32 num_keys = input->char_count;
	while (num_keys--)
	{
		u8 key = input->char_queue[num_keys];
		if (ToUpper(key) == 'W')
			return (0);
		else
		if (ToUpper(key) == 'D')
			return (1);
		else
		if (ToUpper(key) == 'S')
			return (2);
		else
		if (ToUpper(key) == 'A')
			return (3);
	}
	return -1;
}

fn b32 IsKeyPressed(const client_input_t *input, u8 key)
{
	return input->keys[key];
}