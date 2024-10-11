#define _CRT_SECURE_NO_WARNINGS
#include "client.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GL/gl.h>
#include "OpenGL.c"
#include "Win32.c"

static const char *Title = "Project 1.exe";
static const int32_t WindowSize[2] = {1600, 900};

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
		}
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
		Win32GetInput(&input, window);
		for (s32 index = 0; index < NumEnteredChars; index++)
			input.char_queue[index] = EnteredChars[index];
		input.char_count = NumEnteredChars;
		input.gpu_driver_desc = title;

		render_output_t output = {0};
		Host(state, &output, input);

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.086f, 0.086f, 0.086f, 0.0f);
		for (int32_t index = 0; index < output.count; index++)
			OpenGLDispatchBuffer(&output.buffers[index], output.viewports[index]);
		SwapBuffers(hDC);
		NumEnteredChars = 0;
	}
	
	return (0);
}