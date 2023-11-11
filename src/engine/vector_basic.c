#include <string.h>
#include <libdragon.h>

#include "engine/vector.h"

void vector_copy(const f32 *src, f32 *dst, const u8 comp)
{
	memcpy(dst, src, sizeof(f32) * comp);
}

void vector_zero(f32 *vec, const u8 comp)
{
	memset(vec, 0, comp);
}

void vector_print(f32 *x, const u8 comp)
{
	for (int i = 0; i < comp; i++)
		debugf("%f ", x[i]);
	debugf("\n");
}
