#ifndef _T3D_EXT_H_
#define _T3D_EXT_H_

#include <t3d/t3d.h>

/* macro for constructing a vector */
#define T3DVEC3(X, Y, Z) ((const T3DVec3){ { X, Y, Z } })

void t3d_vec3_zero(T3DVec3 *v);

#endif /* _T3D_EXT_H_ */
