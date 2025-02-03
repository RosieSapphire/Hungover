#include "util.h"
#include "t3d_ext.h"
#include "input.h"

#include "engine/player.h"

#define PLAYER_MOVE_UNITS_PER_SEC (87.4f * (1 + INPUT_GET_BTN(Z, HELD)))

player_t player_init(void)
{
	player_t p;

	p.cam = camera_init(&T3D_VEC3_ZERO, 0.f, 0.f, NULL);

	return p;
}

void player_update(player_t *p, const float dt)
{
	/* angle */
	camera_update(&p->cam, dt);

	/* position */
	T3DVec3 focus, look_dir;
	camera_get_values(&p->cam, NULL, &focus, NULL, 1.f);
	t3d_vec3_diff(&look_dir, &focus, &p->cam.eye);

	const int forw_sign =
		INPUT_GET_BTN(C_UP, HELD) - INPUT_GET_BTN(C_DOWN, HELD);
	const int side_sign =
		INPUT_GET_BTN(C_RIGHT, HELD) - INPUT_GET_BTN(C_LEFT, HELD);
	T3DVec3 forw_move = look_dir;
	T3DVec3 side_move, move;
	t3d_vec3_cross(&side_move, &forw_move, &p->cam.up);
	t3d_vec3_scale(&forw_move, &forw_move, forw_sign);
	t3d_vec3_scale(&side_move, &side_move, side_sign);

	t3d_vec3_add(&move, &forw_move, &side_move);
	t3d_vec3_norm(&move);
	t3d_vec3_scale(&move, &move, PLAYER_MOVE_UNITS_PER_SEC * dt);

	p->cam.eye_old = p->cam.eye;
	t3d_vec3_add(&p->cam.eye, &p->cam.eye, &move);
}

void player_terminate(player_t *p)
{
	camera_terminate(&p->cam);
	p->pos = T3D_VEC3_ZERO;
	p->pos_old = T3D_VEC3_ZERO;
}
