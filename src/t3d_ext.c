#include "t3d_ext.h"

void t3d_vec3_negate(T3DVec3 *dst, const T3DVec3 *src)
{
	for (int i = 0; i < 3; i++) {
		dst->v[i] = -src->v[i];
	}
}

int t3d_raycast_triangle(const T3DVec3 *eye, const T3DVec3 *dir,
			 const T3DVec3 positions[3], float *distance)
{
	T3DVec3 edge1, edge2, p, t, q;

	t3d_vec3_diff(&edge1, positions + 1, positions + 0);
	t3d_vec3_diff(&edge2, positions + 2, positions + 0);
	t3d_vec3_cross(&p, dir, &edge2);

	float det = t3d_vec3_dot(&edge1, &p);
	const float epsilon = 0.001f;

	if (det > -epsilon && det < epsilon)
		return (0);

	float inv_det = 1.0f / det;

	t3d_vec3_diff(&t, eye, positions + 0);
	float u = inv_det * t3d_vec3_dot(&t, &p);

	if (u < 0.0f || u > 1.0f)
		return (0);

	t3d_vec3_cross(&q, &t, &edge1);

	float v = inv_det * t3d_vec3_dot(dir, &q);

	if (v < 0.0f || u + v > 1.0f)
		return (0);

	float dist = inv_det * t3d_vec3_dot(&edge2, &q);

	if (!distance)
		return (dist > epsilon);

	*distance = dist;

	return (dist > epsilon);
}
