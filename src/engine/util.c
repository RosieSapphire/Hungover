#include <GL/gl.h>

#include "engine/config.h"
#include "engine/util.h"

float clampf(float x, float min, float max)
{
	if(x > max)
		return max;

	if(x < min)
		return min;

	return x;
}

float lerpf(float a, float b, float t)
{
	return a + (b - a) * clampf(t, 0, 1);
}

float smoothf(float a, float b, float t)
{
	return lerpf(a, b, t * t * (3 - 2 * t));
}

float wrapf(float x, float max)
{
	while(x > max)
		x -= max;

	return x;
}

void projection_setup(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-CONF_NEAR_PLANE * CONF_ASPECT,
			CONF_NEAR_PLANE * CONF_ASPECT,
			-CONF_NEAR_PLANE, CONF_NEAR_PLANE,
			CONF_NEAR_PLANE, CONF_FAR_PLANE);
}
