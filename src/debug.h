typedef struct
{
	log_t *log;
	command_buffer_t *out; // NOTE(): World-space
	command_buffer_t *out_top; // NOTE(): Screen-space
	const bmfont_t *font;
	v2 print_p;
	char format_buffer[512];
} debug_state_t;

static debug_state_t Debug;

fn void BeginDebugFrame(command_buffer_t *output, command_buffer_t *output_top, const bmfont_t *font, log_t *log);
fn void EndDebugFrame(void);

fn void DebugPoint(v2 p, v4 color);
fn void DebugRect(v2 p, v2 sz, v4 color);
fn void DebugRectOutline(v2 p, v2 sz, v4 color);

fn void DebugLine(v2 from, v2 to, v4 color);
fn void DebugVector(v2 p, v2 v, v4 color);
fn void DebugVectorSmall(v2 p, v2 v, v4 color);

fn void DebugVolume(v2 min, v2 max, v4 color);
fn void DebugVolumeOutline(v2 min, v2 max, v4 color);

fn void DebugCircleOutline(v2 p, f32 radius, v4 color);
fn void DebugText(v2 p, const char *string);

fn void DebugPrint(const char *format, ...);

#define DebugLog(Format, ...) _DebugLog(__func__, __LINE__, Format)
fn void _DebugLog(const char *prefix, s32 line, const char *format, ...);