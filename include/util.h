#pragma once

#include <t3d/t3d.h>

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
void radian_wrap_2pi_dual(float *rad_a_ptr, float *rad_b_ptr);
T3DVec2 joystick_get_clamped(const int8_t stick_x, const int8_t stick_y);

/* T3D custom functions for additional stuff. */
T3DVec3 t3d_vec3_make(const float x, const float y, const float z);
T3DVec3 t3d_vec3_xup(void);
T3DVec3 t3d_vec3_yup(void);
T3DVec3 t3d_vec3_zup(void);
T3DVec3 t3d_vec3_zero(void);
T3DVec3 t3d_vec3_one(void);
T3DVec3 t3d_vec3_scale(const T3DVec3 *inp, const float mul);
void debugf_t3d_vec3(const char *name, const T3DVec3 *vec);
