#include "util.h"

float lerpf(const float a, const float b, const float t)
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

T3DVec3 t3d_vec3_one(void)
{
        return t3d_vec3_make(1.f, 1.f, 1.f);
}

T3DVec3 t3d_vec3_scale(const T3DVec3 *inp, const float mul)
{
        int i;
        T3DVec3 out;

        out = *inp;
        for (i = 0; i < 3; ++i)
                out.v[i] *= mul;

        return out;
}

void debugf_t3d_vec3(const char *name, const T3DVec3 *vec)
{
        debugf("%s=(%f, %f, %f)\n", name, vec->v[0], vec->v[1], vec->v[2]);
}
