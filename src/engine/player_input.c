#include "engine/util.h"
#include "engine/vector.h"
#include "engine/player.h"

#define ACCEL 0.1f
#define DECEL 0.1f
#define FRICTION 4

void player_friction_update(player_t *p)
{
	const f32 speed = vector_magnitude(p->vel, 3);

	if (speed <= 0.0f)
	{
		vector_zero(p->vel, 3);
		return;
	}

	f32 newspeed = speed - (speed * DECEL * FRICTION);

	if (newspeed < 0.001f)
		newspeed = 0;
	newspeed /= speed;
	vector_scale(p->vel, newspeed, 3);
}

void player_acceleration_update(player_t *p, struct update_parms uparms)
{
	int cx = uparms.held.c_left - uparms.held.c_right;
	int cy = uparms.held.c_up - uparms.held.c_down;
	f32 forw[3] = {
		cosf(p->cam.yaw) * cy,
		sinf(p->cam.yaw) * cy,
		0
	};
	f32 side[3] = {
		sinf(-p->cam.yaw) * cx,
		cosf(-p->cam.yaw) * cx,
		0
	};
	f32 move[3];

	vector_add(forw, side, move, 3);
	vector_normalize(move, 3);
	vector_scale(move, ACCEL, 3);
	vector_add(p->vel, move, p->vel, 3);
}

void player_look_update(player_t *p, struct update_parms uparms)
{
	f32 stick_x = ((f32)uparms.stick.stick_x / 85.0f);
	f32 stick_y = ((f32)uparms.stick.stick_y / 85.0f);

	if (fabsf(stick_x) < 0.1f)
		stick_x = 0;
	if (fabsf(stick_y) < 0.1f)
		stick_y = 0;

	p->cam.yaw_last = p->cam.yaw;
	p->cam.yaw -= stick_x * 0.2f;
	p->cam.pitch_last = p->cam.pitch;
	p->cam.pitch -= stick_y * 0.2f;

	p->cam.yaw_smooth = lerpf(p->cam.yaw_smooth, p->cam.yaw, 0.5f);
	if (fabsf(p->cam.yaw_smooth - p->cam.yaw) < 0.01f)
		p->cam.yaw_smooth = p->cam.yaw;

	p->cam.pitch_smooth = lerpf(p->cam.pitch_smooth, p->cam.pitch, 0.5f);
	if (fabsf(p->cam.pitch_smooth - p->cam.pitch) < 0.01f)
		p->cam.pitch_smooth = p->cam.pitch;
}

void player_pos_and_focus_update(player_t *p)
{
	vector_copy(p->cam.eye, p->cam.eye_last, 3);
	vector_add(p->cam.eye, p->vel, p->cam.eye, 3);
	vector_copy(p->cam.foc, p->cam.foc_last, 3);
	camera_get_focus(&p->cam);
}
