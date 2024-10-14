typedef struct
{
	f64 timestamp;
	char text[256];
} log_line_t;

typedef struct
{
	f64 time;
	s32 offset;
	log_line_t lines[32];
} log_t;

fn void LogLnVariadic(log_t *Log, const char *format, va_list args);
fn void LogLn(log_t *Log, const char *format, ...);
fn log_line_t *GetLogLn(log_t *Log, s32 offset);
fn void MessageLog(command_buffer_t *out, const bmfont_t *font, v2 p, log_t *Log, f32 dt);