#define _CRT_SECURE_NO_WARNINGS
#include "client.h"
#include <GLFW/glfw3.h>
#include "opengl.c"
#include <stdio.h>

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
		glfwWindowHint(GLFW_SAMPLES, 8);
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
					input.keys[key_code_alt] = (u8)glfwGetKey(window, GLFW_KEY_LEFT_ALT);
					input.keys[key_code_shift] = (u8)glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
					input.keys[key_code_space] = (u8)glfwGetKey(window, GLFW_KEY_SPACE);
					input.keys[key_code_tab] = (u8)glfwGetKey(window, GLFW_KEY_TAB);
					input.keys[key_code_f1] = (u8)glfwGetKey(window, GLFW_KEY_F1);
					input.keys[key_code_f2] = (u8)glfwGetKey(window, GLFW_KEY_F2);
					input.keys[key_code_f3] = (u8)glfwGetKey(window, GLFW_KEY_F3);
					input.keys[key_code_f4] = (u8)glfwGetKey(window, GLFW_KEY_F4);
					input.keys[key_code_f5] = (u8)glfwGetKey(window, GLFW_KEY_F5);

					//numerics manually
					input.keys[key_code_0] = (u8)glfwGetKey(window, '0');
					input.keys[key_code_1] = (u8)glfwGetKey(window, '1');
					input.keys[key_code_2] = (u8)glfwGetKey(window, '2');
					input.keys[key_code_3] = (u8)glfwGetKey(window, '3');
					input.keys[key_code_4] = (u8)glfwGetKey(window, '4');
					input.keys[key_code_5] = (u8)glfwGetKey(window, '5');
					input.keys[key_code_6] = (u8)glfwGetKey(window, '6');
					input.keys[key_code_7] = (u8)glfwGetKey(window, '7');
					input.keys[key_code_8] = (u8)glfwGetKey(window, '8');
					input.keys[key_code_9] = (u8)glfwGetKey(window, '9');

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
						OpenGLDispatchBuffer(&output.buffers[index], output.viewports[index]);
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