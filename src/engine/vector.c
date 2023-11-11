#include <math.h>
#include <string.h>
#include <libdragon.h>

#include "engine/util.h"
#include "engine/vector.h"

void vector_copy(const f32 *src, f32 *dst, const u8 comp)
{
	memcpy(dst, src, sizeof(f32) * comp);
}

void vector_zero(f32 *vec, const u8 comp)
{
	memset(vec, 0, comp);
}

void vector_add(const f32 *a, const f32 *b, f32 *c, const u8 comp)
{
	for (int i = 0; i < comp; i++)
		c[i] = a[i] + b[i];
}

void vector_sub(const f32 *a, const f32 *b, f32 *c, const u8 comp)
{
	for (int i = 0; i < comp; i++)
		c[i] = a[i] - b[i];
}

void vector_scale(f32 *x, f32 s, const u8 comp)
{
	for (int i = 0; i < comp; i++)
		x[i] = x[i] * s;
}

void vector_lerp(const f32 *a, const f32 *b, f32 t, f32 *o, const u8 comp)
{
	for (int i = 0; i < comp; i++)
		o[i] = lerpf(a[i], b[i], t);
}

void vector_smooth(f32 *a, f32 *b, f32 t, f32 *o, const u8 comp)
{
	for (int i = 0; i < comp; i++)
		o[i] = smoothf(a[i], b[i], t);
}

f32 vector_dot(const f32 *a, const f32 *b, const u8 comp)
{
	f32 total = 0.0f;

	for (int i = 0; i < comp; i++)
		total += a[i] * b[i];

	return (total);
}

f32 vector_magnitude_sqr(f32 *x, const u8 comp)
{
	return (vector_dot(x, x, comp));
}

f32 vector_magnitude(f32 *x, const u8 comp)
{
	return (sqrtf(vector_magnitude_sqr(x, comp)));
}

f32 vector_normalize(f32 *x, const u8 comp)
{
	f32 mag = vector_magnitude(x, comp);

	if (!mag)
		return (0.0f);

	for (int i = 0; i < comp; i++)
		x[i] /= mag;

	return (mag);
}

void vector_print(f32 *x, const u8 comp)
{
	for (int i = 0; i < comp; i++)
		debugf("%f ", x[i]);
	debugf("\n");
}
