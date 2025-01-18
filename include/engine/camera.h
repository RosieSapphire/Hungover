#ifndef _ENGINE_CAMERA_H_
#define _ENGINE_CAMERA_H_

#include <t3d/t3d.h>

typedef struct {
	T3DVec3 eye_old, up_old;
	float yaw_deg_old, pitch_deg_old;
	T3DVec3 eye, up;
	float yaw_deg, pitch_deg;
} camera_t;

/*
 * If parameter `up` is NULL, it uses XYZ:(0, 1, 0) by default.
 */
camera_t camera_init(const T3DVec3 *eye, const float yaw_deg,
		     const float pitch_deg, const T3DVec3 *up);
void camera_update(camera_t *c, const int controller_port);

/*
 * `interp` is usually whatever the `subtick` value is for the sake of
 * interpolation. Alternatively you can use 0.f for the PREVIOUS values or
 * 1.f if you want the CURRENT values.
 */
void camera_get_values(const camera_t *c, T3DVec3 *eye, T3DVec3 *focus,
		       T3DVec3 *up, const float interp);
void camera_to_viewport(T3DViewport *vp, const camera_t *c,
			const float subtick);

#endif /* _ENGINE_CAMERA_H_ */
