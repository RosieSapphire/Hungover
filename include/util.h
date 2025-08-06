#pragma once

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

float lerpf(const float a, const float b, const float t);
float clampf(const float x, const float min, const float max);
void radian_wrap_2pi_dual(float *rad_a_ptr, float *rad_b_ptr);
