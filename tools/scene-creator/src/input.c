#include <stdio.h>

#include "input.h"

int input_scroll_y = 0;

#define POLL_KEY(KEY_INPUT, KEY_GLFW)                                       \
	do {                                                                \
		input_new.KEY_INPUT##_down =                                \
			glfwGetKey(glfw_window, KEY_GLFW);                  \
		input_new.KEY_INPUT##_press = input_new.KEY_INPUT##_down && \
					      (input_old.KEY_INPUT##_down ^ \
					       input_new.KEY_INPUT##_down); \
	} while (0)

static void input_scroll_callback(__attribute__((unused))
				  GLFWwindow *glfw_window,
				  __attribute__((unused)) double x_offset,
				  double y_offset)
{
	input_scroll_y = (int)y_offset;
}

input_t input_poll(GLFWwindow *glfw_window, const input_t input_old)
{
	input_t input_new;

	input_scroll_y = 0;
	glfwPollEvents();
	glfwSetScrollCallback(glfw_window, input_scroll_callback);

	/* keys */
	POLL_KEY(w, GLFW_KEY_W);
	POLL_KEY(a, GLFW_KEY_A);
	POLL_KEY(s, GLFW_KEY_S);
	POLL_KEY(d, GLFW_KEY_D);
	POLL_KEY(e, GLFW_KEY_E);
	POLL_KEY(q, GLFW_KEY_Q);
	POLL_KEY(g, GLFW_KEY_G);
	POLL_KEY(x, GLFW_KEY_X);
	POLL_KEY(esc, GLFW_KEY_ESCAPE);
	POLL_KEY(enter, GLFW_KEY_ENTER);
	POLL_KEY(space, GLFW_KEY_SPACE);
	POLL_KEY(shift, GLFW_KEY_LEFT_SHIFT);
	POLL_KEY(f1, GLFW_KEY_F1);
	POLL_KEY(tilde, GLFW_KEY_GRAVE_ACCENT);

	/* mouse */
	for (int i = 0; i < 3; i++) {
		input_new.mb_down[i] = glfwGetMouseButton(glfw_window, i);
		input_new.mb_press[i] =
			(input_new.mb_down[i] &&
			 (input_new.mb_down[i] ^ input_old.mb_down[i]));
	}

	glfwGetCursorPos(glfw_window, &input_new.mouse_x, &input_new.mouse_y);
	input_new.mouse_x_diff = input_new.mouse_x - input_old.mouse_x;
	input_new.mouse_y_diff = input_new.mouse_y - input_old.mouse_y;

	return input_new;
}
