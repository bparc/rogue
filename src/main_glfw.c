#define _CRT_SECURE_NO_WARNINGS
#include "client.h"
#include "external/GLFW/glfw3.h"
#include "opengl.c"

static u8 EnteredChars[8];
static s32 NumEnteredChars;

fn void OnTextEntered(GLFWwindow *window, unsigned int codepoint)
{
	if ((codepoint >= ' ' && codepoint <= 'z') && (NumEnteredChars < ArraySize(EnteredChars)))
		EnteredChars[NumEnteredChars++] = (u8)codepoint;
}

extern int main(void)
{
	if (glfwInit())
	{
		GLFWwindow *window = glfwCreateWindow(1600, 900, "Project1.exe", 0, 0);
		Assert(window);

		glfwMakeContextCurrent(window);
		glfwSetCharCallback(window, OnTextEntered);

		client_t *state = (client_t *)malloc(sizeof(*state));
		if (Startup(state))
		{
			while (1)
			{
				glfwPollEvents();
				if (!glfwWindowShouldClose(window))
				{
					// NOTE(): Read input.
					int width, height;
					double x, y;
					glfwGetCursorPos(window, &x, &y);
					glfwGetWindowSize(window, &width, &height);

					client_input_t input = {0};
					for (int key = '0'; key <= 'Z'; key++)
						input.keys[key] = (u8)glfwGetKey(window, key);
					for (s32 index = 0; index < NumEnteredChars; index++)
						input.char_queue[index] = EnteredChars[index];
					input.char_count = NumEnteredChars;
					NumEnteredChars = 0;

					input.mouse[0] = (f32)x;
					input.mouse[1] = (f32)y;
					input.viewport[0] = (f32)width;
					input.viewport[1] = (f32)height;
					input.time = glfwGetTime();
					
					// NOTE(): Host.
					render_output_t output = {0};
					Host(state, &output, input);

					// NOTE(): Dispatch.
					glClear(GL_COLOR_BUFFER_BIT);
					glClearColor(0.086f, 0.086f, 0.086f, 0.0f);
					for (int32_t index = 0; index < output.count; index++)
						OpenGLDispatchBuffer(&output.buffers[index]);
					glfwSwapBuffers(window);
					continue;
				}
				break;
			}
		}
	}
	return (0);
}

fn void Error(const char *format, ...)
{
	assert(0);
}

fn void _Assert(const char *message, const char *file, const char *function, int32_t line)
{
	assert(0);
}