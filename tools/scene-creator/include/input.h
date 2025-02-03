#ifndef _INPUT_H_
#define _INPUT_H_

#include <GLFW/glfw3.h>

#define MOUSE_BUTTON_COUNT 3

typedef struct {
	/* keys */
	int w_down, w_press;
	int s_down, s_press;
	int a_down, a_press;
	int d_down, d_press;
	int e_down, e_press;
	int q_down, q_press;
	int g_down, g_press;
	int x_down, x_press;
	int esc_down, esc_press;
	int enter_down, enter_press;
	int space_down, space_press;
	int shift_down, shift_press;
	int f1_down, f1_press;
	int tilde_down, tilde_press;

	/* mouse */
	int mb_down[MOUSE_BUTTON_COUNT], mb_press[MOUSE_BUTTON_COUNT];
	double mouse_x, mouse_y;
	double mouse_x_diff, mouse_y_diff;
} input_t;

extern int input_scroll_y;

input_t input_poll(GLFWwindow *glfw_window, const input_t input_old);

#endif /* _INPUT_H_ */
