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