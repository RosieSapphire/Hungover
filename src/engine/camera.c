#include <t3d/t3dmath.h>

#include "util.h"
#include "config.h"
#include "input.h"
#include "t3d_ext.h"

#include "engine/camera.h"

#define CAM_TURN_DEG_PER_SEC 119.3f
#define CAM_TURN_Y_MAX 89.9f
#define CAM_MOVE_UNITS_PER_SEC (87.4f * (1 + INPUT_GET_BTN(Z, HELD)))

// #define CAMERA_DEBUG

camera_t camera_init(const T3DVec3 *eye, const float yaw_deg,
		     const float pitch_deg, const T3DVec3 *up)
{
	camera_t cam;

	cam.eye = cam.eye_old = *eye;
	cam.yaw_deg = cam.yaw_deg_old = yaw_deg;
	cam.pitch_deg = cam.pitch_deg_old = pitch_deg;
	if (!up) {
		cam.up = cam.up_old = T3DVEC3(0.f, 1.f, 0.f);
		return cam;
	}
	cam.up = cam.up_old = *up;

	return cam;
}

void camera_update(camera_t *c, const float dt)
{
	// c->up_old = c->up;

	float stick[2] = { INPUT_GET_STICK(X), INPUT_GET_STICK(Y) };

	/* yaw */
	c->yaw_deg_old = c->yaw_deg;
	c->yaw_deg += stick[0] * CAM_TURN_DEG_PER_SEC * dt;
	while (c->yaw_deg >= 360.f) {
		c->yaw_deg -= 360.f;
		c->yaw_deg_old -= 360.f;
	}
	while (c->yaw_deg <= 0.f) {
		c->yaw_deg += 360.f;
		c->yaw_deg_old += 360.f;
	}

	/* pitch */
	c->pitch_deg_old = c->pitch_deg;
	c->pitch_deg += stick[1] * LOOK_Y_SIGN * CAM_TURN_DEG_PER_SEC * dt;
	if (c->pitch_deg >= CAM_TURN_Y_MAX) {
		c->pitch_deg = CAM_TURN_Y_MAX;
	}
	if (c->pitch_deg <= -CAM_TURN_Y_MAX) {
		c->pitch_deg = -CAM_TURN_Y_MAX;
	}

	/* position */
	T3DVec3 focus, look_dir;
	camera_get_values(c, NULL, &focus, NULL, 1.f);
	t3d_vec3_diff(&look_dir, &focus, &c->eye);

	const int forw_sign =
		INPUT_GET_BTN(C_UP, HELD) - INPUT_GET_BTN(C_DOWN, HELD);
	const int side_sign =
		INPUT_GET_BTN(C_RIGHT, HELD) - INPUT_GET_BTN(C_LEFT, HELD);
	T3DVec3 forw_move = look_dir;
	T3DVec3 side_move, move;
	t3d_vec3_cross(&side_move, &forw_move, &c->up);
	t3d_vec3_scale(&forw_move, &forw_move, forw_sign);
	t3d_vec3_scale(&side_move, &side_move, side_sign);

	t3d_vec3_add(&move, &forw_move, &side_move);
	t3d_vec3_norm(&move);
	t3d_vec3_scale(&move, &move, CAM_MOVE_UNITS_PER_SEC * dt);

	c->eye_old = c->eye;
	t3d_vec3_add(&c->eye, &c->eye, &move);

#ifdef CAMERA_DEBUG
	debugf("CAMERA\n");
	debugf("\tPitch: %f\n\tCam Yaw: %f\n", c->yaw_deg, c->pitch_deg);
	debugf("\tPos: (%f, %f, %f)\n", c->eye.v[0], c->eye.v[1], c->eye.v[2]);
	debugf("\tLookdir: (%f, %f, %f)\n", look_dir.v[0], look_dir.v[1],
	       look_dir.v[2]);
	debugf("\tMove: (%f, %f, %f)\n", move.v[0], move.v[1], move.v[2]);
#endif
}

void camera_terminate(camera_t *c)
{
	t3d_vec3_zero(&c->eye_old);
	t3d_vec3_zero(&c->up_old);
	t3d_vec3_zero(&c->eye);
	t3d_vec3_zero(&c->up);
	c->yaw_deg_old = c->pitch_deg_old = c->yaw_deg = c->pitch_deg = 0;
}

void camera_get_values(const camera_t *c, T3DVec3 *eye, T3DVec3 *focus,
		       T3DVec3 *up, const float interp)
{
	if (eye) {
		t3d_vec3_lerp(eye, &c->eye_old, &c->eye, interp);
	}
	if (focus) {
		T3DVec3 focus_old = {
			{ cosf(T3D_DEG_TO_RAD(c->yaw_deg_old)) *
				  cosf(T3D_DEG_TO_RAD(c->pitch_deg_old)),
			  T3D_DEG_TO_RAD(c->pitch_deg_old),
			  sinf(T3D_DEG_TO_RAD(c->yaw_deg_old)) *
				  cosf(T3D_DEG_TO_RAD(c->pitch_deg_old)) }
		};
		T3DVec3 focus_new = {
			{ cosf(T3D_DEG_TO_RAD(c->yaw_deg)) *
				  cosf(T3D_DEG_TO_RAD(c->pitch_deg)),
			  T3D_DEG_TO_RAD(c->pitch_deg),
			  sinf(T3D_DEG_TO_RAD(c->yaw_deg)) *
				  cosf(T3D_DEG_TO_RAD(c->pitch_deg)) }
		};
		t3d_vec3_add(&focus_old, &focus_old, &c->eye_old);
		t3d_vec3_add(&focus_new, &focus_new, &c->eye);
		t3d_vec3_lerp(focus, &focus_old, &focus_new, interp);
	}
	if (up) {
		t3d_vec3_lerp(up, &c->up_old, &c->up, interp);
	}
}

void camera_to_viewport(T3DViewport *vp, const camera_t *c, const float subtick)
{
	T3DVec3 eye, focus, up;
	camera_get_values(c, &eye, &focus, &up, subtick);
	t3d_viewport_look_at(vp, &eye, &focus, &up);
}
