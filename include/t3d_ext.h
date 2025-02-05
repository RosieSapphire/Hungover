#ifndef _T3D_EXT_H_
#define _T3D_EXT_H_

#include <t3d/t3d.h>

/* macro for constructing a vector */
#define T3D_VEC3(X, Y, Z) ((const T3DVec3){ { X, Y, Z } })

/* constants for T3DVec3 values */
#define T3D_VEC3_ZERO (T3D_VEC3(0, 0, 0))
#define T3D_VEC3_ONE (T3D_VEC3(1, 1, 1))
#define T3D_VEC3_UP (T3D_VEC3(0, 1, 0))

void t3d_vec3_negate(T3DVec3 *dst, const T3DVec3 *src);

int t3d_raycast_triangle(const T3DVec3 *eye, const T3DVec3 *dir,
			 const T3DVec3 *positions[3], float *distance);

#endif /* _T3D_EXT_H_ */
