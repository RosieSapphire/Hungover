#include <math.h>

#include "engine/vector.h"
#include "engine/camera.h"

/**
 * camera_init - Initializes Camera
 * @c: Camera in Question
 */
void camera_init(struct camera *c)
{
	c->pitch = c->yaw = c->pitch_smooth = c->yaw_smooth = 0;
	c->pitch_last = c->yaw_last = 0;
	vector_zero(c->eye_last, 3);
	vector_zero(c->eye, 3);
	vector_zero(c->foc_last, 3);
	c->foc_last[0] = 1;
	vector_copy(c->foc_last, c->foc, 3);
}

/**
 * camera_get_focus - Calculates the Focus Vector for a Camera
 * @c: Camera in Question
 */
void camera_get_focus(struct camera *c)
{
	const float cosp = cosf(c->pitch_smooth);

	vector_copy(c->foc, c->foc_last, 3);
	vector_copy(c->eye, c->foc, 3);
	c->foc[0] += cosf(c->yaw_smooth) * cosp;
	c->foc[1] += sinf(c->yaw_smooth) * cosp;
	c->foc[2] += sinf(c->pitch_smooth);
}
