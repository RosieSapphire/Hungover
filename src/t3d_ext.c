#include "t3d_ext.h"

void t3d_vec3_negate(T3DVec3 *dst, const T3DVec3 *src)
{
	for (u8 i = 0; i < 3; i++) {
		dst->v[i] = -src->v[i];
	}
}

void t3d_quat_negate(T3DQuat *dst, const T3DQuat *src)
{
	for (u8 i = 0; i < 4; i++) {
		dst->v[i] = -src->v[i];
	}
}

void t3d_quat_scale(T3DQuat *dst, const T3DQuat *src, const f32 scale)
{
	for (u8 i = 0; i < 4; i++) {
		dst->v[i] = src->v[i] * scale;
	}
}

void t3d_quat_add(T3DQuat *dst, const T3DQuat *a, const T3DQuat *b)
{
	for (u8 i = 0; i < 4; i++) {
		dst->v[i] = a->v[i] + b->v[i];
	}
}

boolean t3d_raycast_triangle(const T3DVec3 *eye, const T3DVec3 *dir,
			     const T3DVec3 positions[3], f32 *distance)
{
	T3DVec3 edge1, edge2, p, t, q;

	t3d_vec3_diff(&edge1, positions + 1, positions + 0);
	t3d_vec3_diff(&edge2, positions + 2, positions + 0);
	t3d_vec3_cross(&p, dir, &edge2);

	f32 det = t3d_vec3_dot(&edge1, &p);
	const f32 epsilon = 0.001f;

	if (det > -epsilon && det < epsilon) {
		return (0);
	}

	f32 inv_det = 1.0f / det;

	t3d_vec3_diff(&t, eye, positions + 0);
	f32 u = inv_det * t3d_vec3_dot(&t, &p);

	if (u < 0.0f || u > 1.0f) {
		return (0);
	}

	t3d_vec3_cross(&q, &t, &edge1);

	f32 v = inv_det * t3d_vec3_dot(dir, &q);

	if (v < 0.0f || u + v > 1.0f) {
		return (0);
	}

	f32 dist = inv_det * t3d_vec3_dot(&edge2, &q);

	if (!distance) {
		return (dist > epsilon);
	}

	*distance = dist;

	return (dist > epsilon);
}
