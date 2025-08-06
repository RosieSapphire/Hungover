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

/* VEC 2 */
T3DVec2 t3d_vec2_make(const float x, const float y)
{
        T3DVec2 v;

        v.v[0] = x;
        v.v[1] = y;

        return v;
}

T3DVec2 t3d_vec2_xup(void)
{
        return t3d_vec2_make(1.f, 0.f);
}

T3DVec2 t3d_vec2_yup(void)
{
        return t3d_vec2_make(0.f, 1.f);
}

T3DVec2 t3d_vec2_zero(void)
{
        return t3d_vec2_make(0.f, 0.f);
}

T3DVec2 t3d_vec2_one(void)
{
        return t3d_vec2_make(1.f, 1.f);
}

T3DVec2 t3d_vec2_scale(const T3DVec2 *inp, const float mul)
{
        int i;
        T3DVec2 out;

        out = *inp;
        for (i = 0; i < 2; ++i)
                out.v[i] *= mul;

        return out;
}

float t3d_vec2_dot(const T3DVec2 *a, const T3DVec2 *b)
{
        float dot;
        uint8_t i;

        dot = 0.f;
        for (i = 0; i < 2; ++i)
                dot += a->v[i] * b->v[i];

        return dot;
}

float t3d_vec2_get_length(const T3DVec2 *v)
{
        return sqrtf(t3d_vec2_dot(v, v));
}

T3DVec2 t3d_vec2_normalize(const T3DVec2 *v)
{
        T3DVec2 vec;
        float mag;
        uint8_t i;

        vec = *v;
        mag = t3d_vec2_get_length(v);
        if (mag == 0.f || mag == 1.f)
                return vec;

        for (i = 0; i < 2; ++i)
                vec.v[i] /= mag;

        return vec;
}

void debugf_t3d_vec2(const char *name, const T3DVec2 *vec)
{
        debugf("%s=(%f, %f)\n", name, vec->v[0], vec->v[1]);
}

/* VEC 3 */
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

T3DVec3 t3d_vec3_normalize(const T3DVec3 *v)
{
        T3DVec3 vec;
        float mag;
        uint8_t i;

        vec = *v;
        mag = t3d_vec3_len(v);
        if (mag == 0.f || mag == 1.f)
                return vec;

        for (i = 0; i < 3; ++i)
                vec.v[i] /= mag;

        return vec;
}

void debugf_t3d_vec3(const char *name, const T3DVec3 *vec)
{
        debugf("%s=(%f, %f, %f)\n", name, vec->v[0], vec->v[1], vec->v[2]);
}
