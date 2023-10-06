#ifndef ENGINE_VECTOR_H_
#define ENGINE_VECTOR_H_

void vector_copy(const float *src, float *dst, int comp);
void vector_zero(float *vec, int comp);
void vector_add(const float *a, const float *b, float *c, const int comp);
void vector_sub(const float *a, const float *b, float *c, const int comp);
void vector_scale(float *x, float s, int comp);
void vector_lerp(const float *a, const float *b, float t, float *o, int comp);
void vector_smooth(float *a, float *b, float t, float *o, int comp);
float vector_dot(const float *a, const float *b, const int comp);
float vector_magnitude_sqr(float *x, int comp);
float vector_magnitude(float *x, int comp);
float vector_normalize(float *x, int comp);
void vector_print(float *x, int comp);

#endif /* ENGINE_VECTOR_H_ */
