#include "t3d_ext.h"

float t3d_lerpf(const float a, const float b, const float t)
{
        return a + (b - a) * t;
}

T3DVec3 t3d_vec3_make(const float x, const float y, const float z)
{
        T3DVec3 v;

        v.v[0] = x;
        v.v[1] = y;
        v.v[2] = z;

        return v;
}

T3DVec3 t3d_vec3_xup(void)
{
        return t3d_vec3_make(1.f, 0.f, 0.f);
}

T3DVec3 t3d_vec3_yup(void)
{
        return t3d_vec3_make(0.f, 1.f, 0.f);
}

T3DVec3 t3d_vec3_zup(void)
{
        return t3d_vec3_make(0.f, 0.f, 1.f);
}

T3DVec3 t3d_vec3_zero(void)
{
        return t3d_vec3_make(0.f, 0.f, 0.f);
}
