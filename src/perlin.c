#include <stdint.h>

#include "perlin.h"

static T3DVec2 perlin_vector_random(const int xi, const int yi)
{
        uint32_t w, s, a, b;
        float random;
        T3DVec2 ranvec;

        w = 8 * sizeof(w);
        s = w >> 1;
        b = yi;

        a = xi * 3284157443;
        b ^= ((a << s) | (a >> (w - s)));
        b *= 1911520717;
        a ^= ((b << s) | (b >> (w - s)));
        a *= 2048419325;

        random = a * (M_PI / ~(~0u >> 1));
        ranvec = t3d_vec2_make(sinf(random), cosf(random));

        return ranvec;
}

static float perlin_dot_grid_gradient(const int xi, const int yi,
                                      const float xf, const float yf)
{
        T3DVec2 v, d;

        v = perlin_vector_random(xi, yi);
        d = t3d_vec2_make(xf - (float)xi, yf - (float)yi);

        return t3d_vec2_dot(&d, &v);
}

static float perlin_interpolate_cube(const float a, const float b,
                                     const float t)
{
        return a + (b - a) * (3.f - t * 2.f) * t * t;
}

float perlinf(const float x, const float y)
{
        int x0, y0, x1, y1;
        float sx, sy, n0, n1, ix0, ix1, value;

        x0 = (int)x;
        y0 = (int)y;
        x1 = x0 + 1;
        y1 = y0 + 1;

        sx = x - (float)x0;
        sy = y - (float)y0;

        n0 = perlin_dot_grid_gradient(x0, y0, x, y);
        n1 = perlin_dot_grid_gradient(x1, y0, x, y);
        ix0 = perlin_interpolate_cube(n0, n1, sx);

        n0 = perlin_dot_grid_gradient(x0, y1, x, y);
        n1 = perlin_dot_grid_gradient(x1, y1, x, y);
        ix1 = perlin_interpolate_cube(n0, n1, sx);

        value = perlin_interpolate_cube(ix0, ix1, sy);

        return value;
}
