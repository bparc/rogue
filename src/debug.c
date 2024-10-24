
fn void BeginDebugFrame(command_buffer_t *output, command_buffer_t *output_top, const bmfont_t *font, log_t *log)
{
	Assert((Debug.out == NULL));
	Debug.font = font;
	Debug.out = output;
	Debug.out_top = output_top;
	Debug.log = log;
	
	Debug.print_p = V2(10.0f, 8.0f);
}

fn void EndDebugFrame(void)
{
	#if 0
	DrawRect(Debug.out, V2(10.0f, 10.0f),
		V2(500.0f, 200.0f), V4(0.0f, 0.0f, 0.0f, 0.2f));
	#endif
	Assert(Debug.out);
	Debug.out = NULL;
}

fn void DebugPoint(v2 p, v4 color)
{
	DrawRectCentered(Debug.out, p, V2(5.0f, 5.0f), color);
}

fn void DebugRect(v2 p, v2 sz, v4 color)
{
	DrawRect(Debug.out, p, sz, color);
}

fn void DebugRectOutline(v2 p, v2 sz, v4 color)
{
	DrawRectOutline(Debug.out, p, sz, color);
}

fn void DebugLine(v2 from, v2 to, v4 color)
{
	DrawLine(Debug.out, from, to, color);
}

fn void DebugVector(v2 p, v2 v, v4 color)
{
	f32 scale = 40.0f;
	v2 n = Normalize(v);
	v2 np = V2(n.y, -n.x);
	np = Scale(np, 10.0f);
	
	v2 a = Add(p, Scale(n, scale));
	v2 b = Add(p, Scale(n, scale * 0.3f));
	v2 c = Sub(b, np);
	v2 d = Add(b, np);
	DebugLine(p, b, color);
	DebugLine(a, c, color);
	DebugLine(a, d, color);
	DebugLine(c, d, color);
}

fn void DebugVectorSmall(v2 p, v2 v, v4 color)
{
	Assert(0);
}

fn void DebugCircleOutline(v2 p, f32 radius, v4 color)
{
	DrawCircleOutline(Debug.out, p, radius, color);
}

fn void DebugVolume(v2 min, v2 max, v4 color)
{
	DebugRect(min, Sub(max, min), color);
}

fn void DebugVolumeOutline(v2 min, v2 max, v4 color)
{
	DebugRectOutline(min, Sub(max, min), color);
}

fn void DebugText(v2 p, const char *string)
{
	DrawText(Debug.out_top, Debug.font, p, string, Yellow());
}

fn void DebugPrint(const char *format, ...)
{
	va_list args = {0};
	va_start(args, format);
	vsprintf(Debug.format_buffer, format, args);
	va_end(args);
	DebugText(Debug.print_p, Debug.format_buffer);
	Debug.print_p.y += 25.0f;
}

fn void _DebugLog(const char *prefix, s32 line, const char *format, ...)
{
	va_list args = {0};
	va_start(args, format);
	sprintf(Debug.format_buffer, "%i %s(): %s", line, prefix, format);
	LogLnVariadic(Debug.log, Debug.format_buffer, args);
	va_end(args);
}