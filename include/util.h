#pragma once

#include <t3d/t3d.h>

#define JOYSTICK_MAG_MAX 60
#define JOYSTICK_MAG_MIN 6

#define U8ARR_TO_U32PACK(U8A) \
        (U8A[0] << 24) | (U8A[1] << 16) | (U8A[2] << 8) | (U8A[3] << 0)
#define U32PACK_TO_U8ARR(U8ARROUT, U32P) \
        do { \
                U8ARROUT[0] = (U32P & 0xFF000000) >> 24; \
                U8ARROUT[1] = (U32P & 0x00FF0000) >> 16; \
                U8ARROUT[2] = (U32P & 0x0000FF00) >> 8; \
                U8ARROUT[3] = (U32P & 0x000000FF) >> 0; \
        } while (0)

typedef struct {
        float v[2];
} T3DVec2;

float lerpf(const float a, const float b, const float t);
float clampf(const float x, const float min, const float max);
void radian_wrap_2pi_dual(float *rad_a_ptr, float *rad_b_ptr);

/* T3D custom functions for additional stuff. */
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
float t3d_vec3_get_length(const T3DVec3 *v);
T3DVec3 t3d_vec3_normalize(const T3DVec3 *v);
void debugf_t3d_vec3(const char *name, const T3DVec3 *vec);
