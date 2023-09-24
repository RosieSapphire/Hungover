#include <string.h>
#include <math.h>

#include "engine/util.h"
#include "engine/vector.h"

void vector_copy(const float *src, float *dst)
{
	memcpy(dst, src, sizeof(float) * 3);
}

void vector_zero(float *vec)
{
	memset(vec, 0, 3);
}

void vector_add(float *a, float *b, float *c)
{
	for(int i = 0; i < 3; i++)
		c[i] = a[i] + b[i];
}

void vector_scale(float *x, float s)
{
	for(int i = 0; i < 3; i++)
		x[i] = x[i] * s;
}

void vector_lerp(const float *a, const float *b, float t, float *o)
{
	for(int i = 0; i < 3; i++)
		o[i] = lerpf(a[i], b[i], t);
}

void vector_smooth(float *a, float *b, float t, float *o)
{
	for(int i = 0; i < 3; i++)
		o[i] = smoothf(a[i], b[i], t);
}

float vector_dot(float *a, float *b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float vector_magnitude_sqr(float *x)
{
	return vector_dot(x, x);
}

float vector_magnitude(float *x)
{
	return sqrtf(vector_magnitude_sqr(x));
}

float vector_normalize(float *x)
{
	float mag = vector_magnitude(x);
	if(!mag)
		return 0.0f;

	for(int i = 0; i < 3; i++)
		x[i] /= mag;

	return mag;
}
