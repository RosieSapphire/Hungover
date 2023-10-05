#include <GL/gl.h>

#include "engine/config.h"
#include "engine/vector.h"
#include "engine/util.h"

float clampf(float x, float min, float max)
{
	if(x > max)
		return max;

	if(x < min)
		return min;

	return x;
}

int clampi(int x, int min, int max)
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

void quat_lerp(const float *a, const float *b, float *c, const float t)
{
	float out_scale = (vector_dot(a, b, 4) >= 0) ? 1.0f : -1.0f;
	for(int i = 0; i < 4; i++)
		c[i] = ((1 - t) * a[i]) + (out_scale * t * b[i]);

	vector_normalize(c, 4);
}

void pos_from_mat(const float *mat, float *pos)
{
	pos[0] = mat[12];
	pos[1] = mat[13];
	pos[2] = mat[14];
}
