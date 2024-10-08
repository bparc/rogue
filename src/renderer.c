fn command_buffer_t PushCommandBuffer(memory_t *memory, s32 size)
{
	command_buffer_t result = {0};
	result.size = size;
	result.commands = (command_t *)Push(memory, sizeof(*result.commands) * result.size);
	result.memory = PartitionMemory(memory, KB(8));
	return result;
}

fn command_t *PushCommand(command_buffer_t *buffer, command_type_t type)
{
	command_t *result = NULL;
	if (buffer->count < buffer->size)
	{
		result = buffer->commands + buffer->count++;
		result->header.type = type;
	}
	return result;
}

fn void FlushCommandBuffer(command_buffer_t *buffer)
{
	buffer->count = 0;
	FlushMemory(&buffer->memory);
}

fn void DrawVolume(command_buffer_t *buffer, v2 min, v2 max, v4 color)
{
	DrawRect(buffer, min, Sub(max, min), color);
}

fn void DrawVolumeOutline(command_buffer_t *buffer, v2 min, v2 max, v4 color)
{
	DrawRectOutline(buffer, min, Sub(max, min), color);
}

fn void DrawRect(command_buffer_t *buffer, v2 p, v2 sz, v4 color)
{
	command_rect_t *command = (command_rect_t *)PushCommand(buffer, command_t_rect);
	if (command)
	{
		command->min[0] = p.x;
		command->min[1] = p.y;
		command->max[0] = (p.x + sz.x);
		command->max[1] = (p.y + sz.y);
		command->color[0] = color.x;
		command->color[1] = color.y;
		command->color[2] = color.z;
		command->color[3] = color.w;
	}
}

fn void DrawBoundsV(command_buffer_t *buffer, v2 min, v2 max, v4 color)
{
	DrawRect(buffer, min, Sub(max, min), color);
}

fn void DrawBounds(command_buffer_t *buffer, bb_t bounds, v4 color)
{
	DrawBoundsV(buffer, bounds.min, bounds.max, color);
}

fn void DrawRectOutline(command_buffer_t *buffer, v2 p, v2 sz, v4 color)
{
	v2 min = p;
	v2 max = Add(min, sz);
	DrawLine(buffer, V2(min.x, min.y), V2(max.x, min.y), color);
	DrawLine(buffer, V2(max.x, min.y), V2(max.x, max.y), color);
	DrawLine(buffer, V2(max.x, max.y), V2(min.x, max.y), color);
	DrawLine(buffer, V2(min.x, min.y), V2(min.x, max.y), color);
}

fn void DrawRectCentered(command_buffer_t *buffer, v2 p, v2 sz, v4 color)
{
	DrawRect(buffer, Sub(p, Scale(sz, 0.5f)), sz, color);
}

fn void DrawPoint(command_buffer_t *buffer, v2 p, v2 sz, v4 color)
{
	DrawRectCentered(buffer, p, V2(5.0f, 5.0f), color);
}

fn void DrawRectOutlineCentered(command_buffer_t *buffer, v2 p, v2 sz, v4 color)
{
	DrawRectOutline(buffer, Sub(p, Scale(sz, 0.5f)), sz, color);
}

fn void DrawBitmap(command_buffer_t *buffer, v2 p, v2 sz, v4 color, const bitmap_t *bitmap)
{
	command_bitmap_t *command = (command_bitmap_t *)PushCommand(buffer, command_t_bitmap);
	if (command)
	{
		command->min[0] = p.x;
		command->min[1] = p.y;
		command->max[0] = (p.x + sz.x);
		command->max[1] = (p.y + sz.y);
		command->color[0] = color.x;
		command->color[1] = color.y;
		command->color[2] = color.z;
		command->color[3] = color.w;
		command->bitmap = bitmap;
	}
}

fn void DrawLine(command_buffer_t *buffer, v2 from, v2 to, v4 color)
{
	command_line_t *command = (command_line_t *)PushCommand(buffer, command_t_line);
	if (command)
	{
		command->from[0] = from.x;
		command->from[1] = from.y;
		command->to[0] = to.x;
		command->to[1] = to.y;
		command->color[0] = color.x;
		command->color[1] = color.y;
		command->color[2] = color.z;
		command->color[3] = color.w;
	}
}

fn void DrawLineLoop(command_buffer_t *out, v2 *points, s32 count, v4 color)
{
	for (s32 index = 0; index < (count - 1); index++)
		DrawLine(out, points[index], points[index + 1], color);
	DrawLine(out, points[count - 1], points[0], color);
}

fn void DrawCircleOutline(command_buffer_t *buffer, v2 p, f32 radius, v4 color)
{
	const s32 segment_count = 32;
	for (s32 index = 0;
		index < segment_count;
		index++)
	{
		f32 T1 = (f32)(index + 0) / (f32)segment_count;
		f32 T2 = (f32)(index + 1) / (f32)segment_count;
		f32 X1 = radius * Sine(T1);
		f32 Y1 = radius * Cosine(T1);
		f32 X2 = radius * Sine(T2);
		f32 Y2 = radius * Cosine(T2);
		v2 A = Add(p, V2(X1, Y1));
		v2 B = Add(p, V2(X2, Y2));
		DrawLine(buffer, A, B, color);
	}
}

fn void DrawText(command_buffer_t *buffer, const bmfont_t *font, v2 p, const char string[], v4 color)
{
	command_text_t *command = (command_text_t *)PushCommand(buffer, command_t_text);
	if (command)
	{
		command->position = p;
		command->length = StringLength(string);
		command->string = (u8 *)Push(&buffer->memory, (command->length + 1));
		command->font = font;
		command->color = color;
		if (command->string)
		{
			for (s32 index = 0; index < command->length; index++)
				command->string[index] = string[index];
			command->string[command->length] = '\0';
		}
	}	
}

fn void DrawQuad(command_buffer_t *buffer, v2 a, v2 b, v2 c, v2 d, v4 color)
{
	command_quad_t *command = (command_quad_t *)PushCommand(buffer, command_t_quad);
	if (command)
	{
		command->points[0] = a;
		command->points[1] = b;
		command->points[2] = c;
		command->points[3] = d;
		command->color[0] = color.x;
		command->color[1] = color.y;
		command->color[2] = color.z;
		command->color[3] = color.w;
	}
}

fn void DrawQuadv(command_buffer_t *buffer, v2 points[4], v4 color)
{
	DrawQuad(buffer, points[0], points[1], points[2], points[3], color);
}

fn void PushRenderOutput(render_output_t *output, const command_buffer_t buffer)
{
	Assert(output->count < ArraySize(output->buffers));
	output->buffers[output->count++] = buffer;
}