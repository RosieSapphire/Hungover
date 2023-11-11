#include "engine/util.h"
#include "engine/vector.h"

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
