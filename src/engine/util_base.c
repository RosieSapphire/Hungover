#include <GL/gl.h>

#include "engine/config.h"
#include "engine/vector.h"
#include "engine/util.h"

f32 clampf(f32 x, f32 min, f32 max)
{
	if (x > max)
		return (max);

	if (x < min)
		return (min);

	return (x);
}

int clampi(int x, int min, int max)
{
	if (x > max)
		return (max);

	if (x < min)
		return (min);

	return (x);
}

f32 lerpf(f32 a, f32 b, f32 t)
{
	return (a + (b - a) * clampf(t, 0, 1));
}

f32 smoothf(f32 a, f32 b, f32 t)
{
	return (lerpf(a, b, t * t * (3 - 2 * t)));
}

f32 wrapf(f32 x, f32 max)
{
	while (x > max)
		x -= max;

	return (x);
}
