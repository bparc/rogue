#include <strsafe.h>

fn double Win32GetTime(void)
{
	double result = 0;

	LARGE_INTEGER frequency = {};
	if (QueryPerformanceFrequency(&frequency))
	{
		LARGE_INTEGER time = {};
		if (QueryPerformanceCounter(&time))
		{
			result = ((double)time.QuadPart / (double)frequency.QuadPart);
		}
	}
	return result;
}

fn int32_t Win32CalculateWindowRectangleSize(DWORD style, int32_t *x, int32_t *y)
{
	RECT rectangle = {0, 0, *x, *y};
	if (AdjustWindowRect(&rectangle, style, 0))
	{
		*x = (rectangle.right - rectangle.left);
		*y = (rectangle.bottom - rectangle.top);
		return(1);
	}
	return (0);
}

fn void Error(const char *format, ...)
{
	va_list args = {0};
	va_start(args, format);
	char message[512] = "";
	StringCbVPrintfA(message, ArraySize(message), format, args);
	va_end(args);
	
	MessageBox(0, message, 0, MB_ICONERROR);
	ExitProcess(1);
}

fn void _Assert(const char *message, const char *file, const char *function, int32_t line)
{
	Error("Assertion failed! \n\nProgram: %s\nLine: %i\n\nFunction: %s\nExpression: %s", file, line, function, message);
}

fn void Win32GetInput(client_input_t *result, HWND window)
{
	ZeroMemory(result, sizeof(*result));
	RECT client_rect = {0};
	GetClientRect(window, &client_rect);
	result->viewport[0] = (float)(client_rect.right - client_rect.left);
	result->viewport[1] = (float)(client_rect.bottom - client_rect.top);
	POINT cursor = {0};
	GetCursorPos(&cursor);
	ScreenToClient(window, &cursor);
	result->mouse[0] = (f32)cursor.x;
	result->mouse[1] = (f32)cursor.y;
	result->mouse_buttons[0] = (GetAsyncKeyState(VK_LBUTTON) < 0);
	result->mouse_buttons[1] = (GetAsyncKeyState(VK_RBUTTON) < 0);
	for (s32 index = '0'; index <= 'Z'; index++)
		result->keys[index] = (GetAsyncKeyState(index) < 0);
}