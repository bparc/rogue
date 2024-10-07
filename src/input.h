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

fn inline v2 GetCursorP(const client_input_t *input)
{
	v2 result = V2((f32)input->mouse[0], (f32)input->mouse[1]);
	return result;
}

fn v2s GetDirectionalInput(const client_input_t *input)
{
	v2s result = {0};
	s32 num_keys = input->char_count;
	while (num_keys--)
	{
		u8 key = input->char_queue[num_keys];
		if (ToUpper(key) == 'D')
			result.x++;
		else
		if (ToUpper(key) == 'A')
			result.x--;
		else
		if (ToUpper(key) == 'W')
			result.y--;
		else
		if (ToUpper(key) == 'S')
			result.y++;

		if (IsZero(result) == false)
			break;
	}
	return result;
}

fn b32 IsKeyPressed(const client_input_t *input, u8 key)
{
	return input->keys[key];
}