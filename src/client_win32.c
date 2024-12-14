#define _CRT_SECURE_NO_WARNINGS
#include "client.c"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GL/gl.h>
#include "Renderer/OpenGL.c"

#include <strsafe.h>

fn double Win32GetTime(void)
{
	double result = 0;

	LARGE_INTEGER frequency = {0};
	if (QueryPerformanceFrequency(&frequency))
	{
		LARGE_INTEGER time = {0};
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

fn void Win32GetInput(client_input_t *result, HWND window, s32 wheel_delta)
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
	result->wheel = wheel_delta;
	result->mouse_buttons[0] = (GetAsyncKeyState(VK_LBUTTON) < 0);
	result->mouse_buttons[1] = (GetAsyncKeyState(VK_RBUTTON) < 0);
	for (s32 index = '0'; index <= 'Z'; index++)
		result->keys[index] = (GetAsyncKeyState(index) < 0);
	result->keys[key_code_alt] = (GetAsyncKeyState(VK_LMENU) < 0);
	result->keys[key_code_shift] = (GetAsyncKeyState(VK_SHIFT) < 0);
	result->keys[key_code_space] = (GetAsyncKeyState(VK_SPACE) < 0);
	result->keys[key_code_tab] = (GetAsyncKeyState(VK_TAB) < 0);
	result->keys[key_code_1] = (GetAsyncKeyState(0x31) < 0);
	result->keys[key_code_2] = (GetAsyncKeyState(0x32) < 0);
	result->keys[key_code_3] = (GetAsyncKeyState(0x33) < 0);
	result->keys[key_code_4] = (GetAsyncKeyState(0x34) < 0);
	result->keys[key_code_5] = (GetAsyncKeyState(0x35) < 0);
	result->keys[key_code_6] = (GetAsyncKeyState(0x36) < 0);
	result->keys[key_code_7] = (GetAsyncKeyState(0x37) < 0);
	result->keys[key_code_8] = (GetAsyncKeyState(0x38) < 0);
	result->keys[key_code_9] = (GetAsyncKeyState(0x39) < 0);
	result->keys[key_code_f1] = (GetAsyncKeyState(VK_F1) < 0);
	result->keys[key_code_f2] = (GetAsyncKeyState(VK_F2) < 0);
	result->keys[key_code_f3] = (GetAsyncKeyState(VK_F3) < 0);
	result->keys[key_code_f4] = (GetAsyncKeyState(VK_F4) < 0);
	result->keys[key_code_f5] = (GetAsyncKeyState(VK_F5) < 0);
	result->keys[key_code_f12] = (GetAsyncKeyState(VK_F12) < 0);
	result->time = Win32GetTime();
}

static const char *Title = "Project 1.exe";
static const int32_t WindowSize[2] = {1600, 900};

static s32 WheelDelta;
static u8 EnteredChars[8];
static s32 NumEnteredChars;

static int32_t Running;
static HCURSOR Cursor;
static HINSTANCE hInstance;
static WNDCLASS WndClass;

static const PIXELFORMATDESCRIPTOR
PixelFormat =
{
	sizeof(PIXELFORMATDESCRIPTOR),
	1,
	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,
	32,
	0, 0, 0, 0, 0, 0,
	0,
	0,
	0,
	0, 0, 0, 0,
	24,
	8,
	0,
	PFD_MAIN_PLANE,
	0,
	0, 0, 0
};

fn LRESULT WINAPI WndProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	LRESULT result = 0;
	switch (uMsg)
	{
	case WM_CLOSE:
		{
			Running = 0;
			ExitProcess(0);
		} break;
	case WM_SETCURSOR:
		{
			SetCursor(Cursor);
		} break;
	case WM_CHAR:
		{
			if (((wParam >= ' ' ) && (wParam <= 'z')) && (NumEnteredChars < ArraySize(EnteredChars)))
				EnteredChars[NumEnteredChars++] = (u8)wParam;
		} break;
	case WM_MOUSEWHEEL:
		{
			WheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		} break;
	default:
		result = DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return result;
}

extern int main(void)
{	
	Cursor = LoadCursor(NULL, IDC_ARROW);
	hInstance = GetModuleHandle(NULL);

	// NOTE(): Window.
	ZeroMemory(&WndClass, sizeof(WndClass));
	WndClass.lpfnWndProc = WndProc;
	WndClass.hInstance = hInstance;
	WndClass.lpszClassName = "My Window Class";
	RegisterClass(&WndClass);
	
	DWORD style = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
	int32_t x = WindowSize[0], y = WindowSize[1];
	Win32CalculateWindowRectangleSize(style, &x, &y);

	HWND window = CreateWindowEx(0, WndClass.lpszClassName, Title, (style | WS_VISIBLE), CW_USEDEFAULT, CW_USEDEFAULT, x, y, 0, 0, hInstance, 0);
	UpdateWindow(window);
	// NOTE(): Initialize OpenGL.
	HGLRC hGLRC = NULL;
	HDC hDC = NULL;
	hDC = GetDC(window);
	const int32_t format = ChoosePixelFormat(hDC, &PixelFormat);
	SetPixelFormat(hDC, format, &PixelFormat);

	hGLRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hGLRC);

	char title[256] = "";
	StringCbPrintfA(title, ArraySize(title), "OpenGL %s %s", glGetString(GL_VERSION), glGetString(GL_RENDERER));
	//SetWindowTextA(window, title);

	// NOTE(): Host.
	client_t *state = (client_t *)VirtualAlloc(0, sizeof(client_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	Running = Startup(state);
	while (Running)
	{
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		
		client_input_t input = {0};
		Win32GetInput(&input, window, WheelDelta);

		for (s32 index = 0; index < NumEnteredChars; index++)
			input.char_queue[index] = EnteredChars[index];
		input.char_count = NumEnteredChars;
		input.gpu_driver_desc = title;
		
		WheelDelta = 0;
		NumEnteredChars = 0;

		render_output_t output = {0};
		Host(state, &output, input);

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.086f, 0.086f, 0.086f, 0.0f);
		for (int32_t index = 0; index < output.count; index++)
			OpenGLDispatchBuffer(&output.buffers[index], output.viewports[index], output.cameras[index]);

		SwapBuffers(hDC);
		
	}
	
	return (0);
}