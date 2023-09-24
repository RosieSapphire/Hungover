#ifndef ENGINE_VECTOR_H_
#define ENGINE_VECTOR_H_

void vector_copy(const float *src, float *dst);
void vector_zero(float *vec);
void vector_add(float *a, float *b, float *c);
void vector_scale(float *x, float s);
void vector_lerp(const float *a, const float *b, float t, float *o);
void vector_smooth(float *a, float *b, float t, float *o);
float vector_dot(float *a, float *b);
float vector_magnitude_sqr(float *x);
float vector_magnitude(float *x);
float vector_normalize(float *x);

#endif /* ENGINE_VECTOR_H_ */
