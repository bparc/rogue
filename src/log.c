fn log_line_t *GetLogLn(log_t *Log, s32 offset)
{
	return &Log->lines[offset % ArraySize(Log->lines)];
}

fn void PushLogLn(log_t *Log, const char *format, ...)
{
	char buffer[256] = "";
	va_list args = {0};
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	log_line_t *Ln = GetLogLn(Log, Log->offset++);
	Ln->timestamp = Log->time;
	char *source = buffer;
	char *at = Ln->text;
	while (*source) // TODO(): Safe string copy...
		*at++ = *source++;
	*at = 0;
}

fn void MessageLog(command_buffer_t *out, const bmfont_t *font, v2 p, log_t *Log, f32 dt)
{
	Log->time += dt;
	p = Add(p, V2(0.0f, 185.0f));

	s32 max = MinS32(Log->offset - 1, ArraySize(Log->lines) - 1);
	s32 min = MaxS32(0, Log->offset - ArraySize(Log->lines));

	for (s32 index = max;
		index >= 0;
		index--)
	{
		log_line_t *line = GetLogLn(Log, min + index);
		f32 lifetime = 3.0f;
		f32 seconds_elapsed = (f32)(Log->time - line->timestamp);
		if (seconds_elapsed <= lifetime)
		{
			f32 t0 = seconds_elapsed / lifetime;
			f32 t1 = Smoothstep(t0, 0.9f);
			f32 t2 = Smoothstep(1.0f - t0, 0.95f);
			f32 t = t1 + t2;
			f32 x = (17.0f + (20.0f * t1)) - (10.0f * t2);
			v4 color = White();
			color.w = 1.0f - t;
			DrawText(out, font, V2(p.x + x, p.y), line->text, color);
			p.y -= 25.0f;
			continue;
		}
		break;
	}
}