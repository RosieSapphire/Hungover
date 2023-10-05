#ifndef ENGINE_UTIL_H_
#define ENGINE_UTIL_H_

#define PI 3.14159265358979323846f
#define PI_HALF 1.57079632679489661923f

#define TO_RADIANS (PI / 180.0f)
#define TO_DEGREES (180.0f / PI)

float clampf(float x, float min, float max);
int clampi(int x, int min, int max);
float lerpf(float a, float b, float t);
float smoothf(float a, float b, float t);
float wrapf(float x, float max);
void projection_setup(void);
void quat_lerp(const float *a, const float *b, float *c, const float t);
void pos_from_mat(const float *mat, float *pos);

#endif /* ENGINE_UTIL_H_ */
