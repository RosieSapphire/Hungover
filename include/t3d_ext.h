#pragma once

#include <t3d/t3d.h>

float t3d_lerpf(const float a, const float b, const float t);
T3DVec3 t3d_vec3_make(const float x, const float y, const float z);
T3DVec3 t3d_vec3_xup(void);
T3DVec3 t3d_vec3_yup(void);
T3DVec3 t3d_vec3_zup(void);
T3DVec3 t3d_vec3_zero(void);
