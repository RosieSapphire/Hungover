#ifndef _T3D_EXT_H_
#define _T3D_EXT_H_

#include <t3d/t3d.h>

/* macro for constructing a vector */
#define T3D_VEC3(X, Y, Z) ((const T3DVec3){ { X, Y, Z } })

/* constants for T3DVec3 values */
#define T3D_VEC3_ZERO (T3D_VEC3(0, 0, 0))
#define T3D_VEC3_ONE (T3D_VEC3(1, 1, 1))

#endif /* _T3D_EXT_H_ */
