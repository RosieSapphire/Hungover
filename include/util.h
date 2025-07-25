#pragma once

#include <t3d/t3d.h>

#define U8_ARR_TO_U32_PACK(U8A) \
        (U8A[0] << 24) | (U8A[1] << 16) | (U8A[2] << 8) | (U8A[3] << 0)
#define U32_PACK_TO_U8_ARR(U8ARROUT, U32P) \
        do { \
                U8ARROUT[0] = (U32P & 0xFF000000) >> 24; \
                U8ARROUT[1] = (U32P & 0x00FF0000) >> 16; \
                U8ARROUT[2] = (U32P & 0x0000FF00) >> 8; \
                U8ARROUT[3] = (U32P & 0x000000FF) >> 0; \
        } while (0)

float lerpf(const float a, const float b, const float t);

/* T3D custom functions for additional stuff. */
T3DVec3 t3d_vec3_make(const float x, const float y, const float z);
T3DVec3 t3d_vec3_xup(void);
T3DVec3 t3d_vec3_yup(void);
T3DVec3 t3d_vec3_zup(void);
T3DVec3 t3d_vec3_zero(void);
T3DVec3 t3d_vec3_one(void);
T3DVec3 t3d_vec3_scale(const T3DVec3 *inp, const float mul);
void debugf_t3d_vec3(const char *name, const T3DVec3 *vec);
