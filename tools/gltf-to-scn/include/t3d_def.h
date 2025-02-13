#ifndef _GLTF_TO_SCN_T3D_DEF_H_
#define _GLTF_TO_SCN_T3D_DEF_H_

#define T3D_RAD_TO_DEG(deg) (deg * 57.29577951289617186798f)

typedef struct {
	float v[3];
} T3DVec3;

typedef struct {
	float v[4];
} T3DQuat;

#endif /* _GLTF_TO_SCN_T3D_DEF_H_ */
