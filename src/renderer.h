typedef enum
{
	command_t_triangle,
	command_t_line,

	// TODO(): Having a "rect" and
	// a "quad" command seems kind of redundant?
	// There should be a "cirle" command, though.
	command_t_rect,
	command_t_quad,
	command_t_text,
	command_t_bitmap,
} command_type_t;

typedef struct
{
	command_type_t type;
} command_header_t;

typedef struct
{
	command_header_t header;
	f32 min[2];
	f32 max[2];
	f32 color[4];
	const bitmap_t *bitmap;
} command_bitmap_t;

typedef struct
{
	command_header_t header;
	f32 min[2];
	f32 max[2];
	f32 color[4];
} command_rect_t;

typedef struct
{
	command_header_t header;
	v2  points[4];
	f32 color[4];
} command_quad_t;

typedef struct
{
	command_header_t header;
	f32 from[2];
	f32 to[2];
	f32 color[4];
} command_line_t;

typedef struct
{
	command_header_t header;
	v2 position;
	v4 color;
	s32 length;
	u8 *string;
	const bmfont_t *font;
} command_text_t;

typedef union
{
	command_header_t header;
	command_line_t line;
	command_rect_t rect;
	command_quad_t quad;
	command_text_t text;
	command_bitmap_t bitmap;
} command_t;

typedef struct
{
	memory_t memory; // NOTE(): A memory pool for storing strings.
	command_t *commands;
	s32 count;
	s32 size;
} command_buffer_t;

fn command_buffer_t PushCommandBuffer(memory_t *memory, s32 size);
fn command_t *PushCommand(command_buffer_t *buffer, command_type_t type);
fn void FlushCommandBuffer(command_buffer_t *buffer);

fn void DrawRect(command_buffer_t *buffer, v2 p, v2 sz, v4 color);
fn void DrawRectCentered(command_buffer_t *buffer, v2 p, v2 sz, v4 color);

fn void DrawRectOutline(command_buffer_t *buffer, v2 p, v2 sz, v4 color);
fn void DrawRectOutlineCentered(command_buffer_t *buffer, v2 p, v2 sz, v4 color);

fn void DrawBounds(command_buffer_t *buffer, bb_t bounds, v4 color);
fn void DrawBoundsV(command_buffer_t *buffer, v2 min, v2 max, v4 color);

fn void DrawVolume(command_buffer_t *buffer, v2 min, v2 max, v4 color);
fn void DrawVolumeOutline(command_buffer_t *buffer, v2 min, v2 max, v4 color);

fn void DrawCircleOutline(command_buffer_t *buffer, v2 p, f32 radius, v4 color);
fn void DrawBitmap(command_buffer_t *buffer, v2 p, v2 sz, v4 color, const bitmap_t *bitmap);

fn void DrawLineLoop(command_buffer_t *out, v2 *points, s32 count, v4 color);
fn void DrawLine(command_buffer_t *buffer, v2 from, v2 to, v4 color);
fn void DrawText(command_buffer_t *buffer, const bmfont_t *font, v2 p, const char string[], v4 color);

fn void DrawPoint(command_buffer_t *buffer, v2 p, v2 sz, v4 color);
fn void DrawQuad(command_buffer_t *buffer, v2 a, v2 b, v2 c, v2 d, v4 color);
fn void DrawQuadv(command_buffer_t *buffer, v2 points[4], v4 color);

typedef struct
{
	int32_t count;
	command_buffer_t buffers[16];
} render_output_t;

fn void PushRenderOutput(render_output_t *output, const command_buffer_t buffer);