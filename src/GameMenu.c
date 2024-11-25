typedef struct
{
	s32 Foo;
} menu_t;

fn void GrowBroccoli(command_buffer_t *Out, v2 At, f32 Angle, f32 LeafScale, s32 Depth, f32 Time)
{
	f32 T = sinf(Time);
	Angle += T * 0.03f;

	s32 MaxDepth = 10;
	f32 LeafSize = 200.0f * LeafScale;
	f32 Theta = 0.07f;

	v2 Dir = V2(Sine(Angle), -Cosine(Angle));

	v2 From = At;
	v2 To = Add(At, Scale(Dir, LeafSize));

	DrawLine(Out, From, To, Green());

	if (Depth < MaxDepth)
	{
		GrowBroccoli(Out, To, Angle - Theta, LeafScale * 0.6f, (Depth + 1), Time);
		GrowBroccoli(Out, To, Angle + Theta, LeafScale * 0.6f, (Depth + 1), Time);
	}
}

fn b32 MenuMode(menu_t *Menu, command_buffer_t *Out, assets_t *Assets, const virtual_controls_t *Cons, f32 Time)
{
	b32 RunGame = false;

	if (WentDown(Cons->confirm))
		RunGame = true;

	f32 BlinkHz = 1.0f;
	if (fmodf(Time, BlinkHz) <= (BlinkHz * 0.5f))
		DrawText(Out, Assets->Font, V2(10.0f, 8.0f), "START (E)", Yellow());

	v2 Viewport = V2(1600.0f, 900.0f);
	v2 BroccoliPos = Scale(Viewport, 0.5f);
	BroccoliPos.y += 100.0f;
	GrowBroccoli(Out, BroccoliPos, 0.0f, 1.0f, 0, Time);

	return (RunGame);
}