#include <string.h>
#include <math.h>

#include "engine/util.h"
#include "engine/vector.h"

void vector_copy(const float *src, float *dst, int comp)
{
	memcpy(dst, src, sizeof(float) * comp);
}

void vector_zero(float *vec, int comp)
{
	memset(vec, 0, comp);
}

void vector_add(const float *a, const float *b, float *c, const int comp)
{
	for(int i = 0; i < comp; i++)
		c[i] = a[i] + b[i];
}

void vector_sub(const float *a, const float *b, float *c, const int comp)
{
	for(int i = 0; i < comp; i++)
		c[i] = a[i] - b[i];
}

void vector_scale(float *x, float s, int comp)
{
	for(int i = 0; i < comp; i++)
		x[i] = x[i] * s;
}

void vector_lerp(const float *a, const float *b, float t, float *o, int comp)
{
	for(int i = 0; i < comp; i++)
		o[i] = lerpf(a[i], b[i], t);
}

void vector_smooth(float *a, float *b, float t, float *o, int comp)
{
	for(int i = 0; i < comp; i++)
		o[i] = smoothf(a[i], b[i], t);
}

float vector_dot(const float *a, const float *b, const int comp)
{
	float total = 0.0f;
	for(int i = 0; i < comp; i++)
		total += a[i] * b[i];

	return total;
}

float vector_magnitude_sqr(float *x, int comp)
{
	return vector_dot(x, x, comp);
}

float vector_magnitude(float *x, int comp)
{
	return sqrtf(vector_magnitude_sqr(x, comp));
}

float vector_normalize(float *x, int comp)
{
	float mag = vector_magnitude(x, comp);
	if(!mag)
		return 0.0f;

	for(int i = 0; i < comp; i++)
		x[i] /= mag;

	return mag;
}
