fn void OpenGLRenderQuad(v2 min, v2 max, v2 uv_min, v2 uv_max, GLuint texture, v4 color)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glColor4fv(&color.x);
	glTexCoord2f(uv_min.x, uv_min.y); glVertex2f(min.x, min.y);
	glTexCoord2f(uv_max.x, uv_min.y); glVertex2f(max.x, min.y);
	glTexCoord2f(uv_max.x, uv_max.y); glVertex2f(max.x, max.y);
	glTexCoord2f(uv_min.x, uv_max.y); glVertex2f(min.x, max.y);
	glEnd();
}

fn void OpenGLDispatchBuffer(const command_buffer_t *buffer, v4 viewport, camera_t camera)
{
	v2 GlobalOffset = V2(0.0f, 0.0f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();
	glLoadIdentity();

	f32 X = viewport.z;
	f32 Y = viewport.w;
	glOrtho(0.0, X, Y, 0.0, 0.0, 1.0);

	f32 scale_factor = GetCameraScaleFactor(&camera);
	glTranslatef(X * 0.5f, Y * 0.5f, 0.0f);
	glScalef(scale_factor, scale_factor, 0.0f);
	glTranslatef(-(X * 0.5f), -(Y * 0.5f), 0.0f);

	for (int32_t index = 0; index < buffer->count; index++)
	{
		const command_t *command = (buffer->commands + index);

		glBindTexture(GL_TEXTURE_2D, 0);
		glPushMatrix();
		glTranslatef(camera.p.x, camera.p.y, 0.0f);
		switch (command->header.type)
		{
		case command_t_rect:
			{
				const command_rect_t *rectangle = (&command->rect);
				glBegin(GL_QUADS);
				glColor4fv(&rectangle->color[0]);
				glVertex2f(rectangle->min[0], rectangle->min[1]);
				glVertex2f(rectangle->max[0], rectangle->min[1]);
				glVertex2f(rectangle->max[0], rectangle->max[1]);
				glVertex2f(rectangle->min[0], rectangle->max[1]);
				glEnd();
			} break;
		case command_t_line:
			{
				const command_line_t *line = (&command->line);
				glBegin(GL_LINES);
				glColor4fv(&line->color[0]);
				glVertex2f(line->from[0], line->from[1]);
				glVertex2f(line->to[0], line->to[1]);
				glEnd();
			} break;
		case command_t_text:
			{
				const command_text_t *text = (&command->text);
				const bmfont_t *font = text->font;

				v2 offset = text->position;
				const u8 *at = text->string;
				while (*at)
				{
					s32 codepoint = *at++;
					bmfont_char_t glyph = font->chars[codepoint];

					v2 bitmap_sz = V2(glyph.width, glyph.height);

					v2 texture_sz = V2((f32)font->bitmap.x, (f32)font->bitmap.y);
					v2 uv_min = V2(glyph.x, glyph.y);
					v2 uv_max = Add(uv_min, bitmap_sz);
					uv_min = Div(uv_min, texture_sz);
					uv_max = Div(uv_max, texture_sz);

					v2 bitmap_p = offset;
					bitmap_p = Add(bitmap_p, V2(glyph.x_offset, glyph.y_offset));

					bitmap_p = Sub(bitmap_p, V2(1.0f, 1.0f));
					OpenGLRenderQuad(bitmap_p, Add(bitmap_p, bitmap_sz), uv_min, uv_max, (GLuint)font->bitmap.handle, V4(0.0f, 0.0f, 0.0f, text->color.w));

					bitmap_p = Add(bitmap_p, V2(1.0f, 1.0f));
					OpenGLRenderQuad(bitmap_p, Add(bitmap_p, bitmap_sz), uv_min, uv_max, (GLuint)font->bitmap.handle, text->color);

					offset.x += glyph.x_advance;
				}
			} break;
		case command_t_bitmap:
			{
				const command_bitmap_t *bitmap = (&command->bitmap);
				Assert(bitmap->bitmap);
				glBindTexture(GL_TEXTURE_2D, (GLuint)bitmap->bitmap->handle);
				glBegin(GL_QUADS);
				glColor4fv(&bitmap->color[0]);
				glTexCoord2f(0.0f, 0.0f); glVertex2f(bitmap->min[0], bitmap->min[1]);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(bitmap->max[0], bitmap->min[1]);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(bitmap->max[0], bitmap->max[1]);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(bitmap->min[0], bitmap->max[1]);
				glEnd();
			} break;
		case command_t_quad:
			{
				const command_quad_t *quad = (&command->quad);
				glBegin(GL_QUADS);
				glColor4fv(&quad->color[0]);
				glVertex2fv(&quad->points[0].x);
				glVertex2fv(&quad->points[1].x);
				glVertex2fv(&quad->points[2].x);
				glVertex2fv(&quad->points[3].x);
				glEnd();
			} break;
		default:
			Assert(0);
		}

		glPopMatrix();
	}

	glPopMatrix();
}

#define STBI_ONLY_PNG
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

fn bitmap_t LoadBitmapFromFile(const char *path)
{
	bitmap_t result = {0};

	u8 *pixels = stbi_load(path, &result.x, &result.y, NULL, 4);
	if (pixels)
	{
		GLuint texture = 0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// TODO(): There should be different texture samplers for "pixel-art" and "high-res" graphics.
		// Furthermore, high quality pixel-art rendering will require adding a custom texture filter via a shader.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result.x, result.y,
			0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		result.scale = V2((f32)result.x, (f32)result.y);
		result.handle = (u32)texture;
		free(pixels);
	}

	return result;
}