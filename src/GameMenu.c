typedef struct
{
	s32 Foo;
} menu_t;

fn b32 MenuMode(menu_t *Menu, command_buffer_t *Out, assets_t *Assets, const virtual_controls_t *Cons, f32 Time)
{
	b32 RunGame = false;

	if (WentDown(Cons->confirm))
		RunGame = true;

	f32 BlinkHz = 1.0f;
	if (fmodf(Time, BlinkHz) <= (BlinkHz * 0.5f))
		DrawText(Out, Assets->Font, V2(10.0f, 8.0f), "START (E)", Yellow());
	return (RunGame);
}