#include <math.h>

#include "t3d_ext.h"
#include "util.h"

float lerpf(const float a, const float b, const float t)
{
        return a + (b - a) * t;
}

float clampf(const float x, const float min, const float max)
{
        return fmaxf(fminf(max, x), min);
}

void radian_wrap_2pi_dual(float *rad_a_ptr, float *rad_b_ptr)
{
        float a, b, limit;

        a = *rad_a_ptr;
        b = *rad_b_ptr;
        limit = 2.f * T3D_PI;

        while (b >= limit) {
                b -= limit;
                a -= limit;
        }

        while (b < 0.f) {
                b += limit;
                a += limit;
        }

        *rad_a_ptr = a;
        *rad_b_ptr = b;
}
