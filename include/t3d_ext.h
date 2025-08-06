#pragma once

#include <t3d/t3d.h>

typedef struct {
        float v[2];
} T3DVec2;

T3DVec2 t3d_vec2_make(const float x, const float y);
T3DVec2 t3d_vec2_xup(void);
T3DVec2 t3d_vec2_yup(void);
T3DVec2 t3d_vec2_zero(void);
T3DVec2 t3d_vec2_one(void);
T3DVec2 t3d_vec2_scale(const T3DVec2 *inp, const float mul);
float t3d_vec2_dot(const T3DVec2 *a, const T3DVec2 *b);
float t3d_vec2_get_length(const T3DVec2 *v);
T3DVec2 t3d_vec2_normalize(const T3DVec2 *v);
void debugf_t3d_vec2(const char *name, const T3DVec2 *vec);

T3DVec3 t3d_vec3_make(const float x, const float y, const float z);
T3DVec3 t3d_vec3_xup(void);
T3DVec3 t3d_vec3_yup(void);
T3DVec3 t3d_vec3_zup(void);
T3DVec3 t3d_vec3_zero(void);
T3DVec3 t3d_vec3_one(void);
T3DVec3 t3d_vec3_scale(const T3DVec3 *inp, const float mul);
float t3d_vec3_dot(const T3DVec3 *a, const T3DVec3 *b);
T3DVec3 t3d_vec3_normalize(const T3DVec3 *v);
void debugf_t3d_vec3(const char *name, const T3DVec3 *vec);
