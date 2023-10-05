#include <GL/gl.h>
#include <GL/glu.h>

#include "engine/vector.h"
#include "engine/util.h"
#include "engine/camera.h"
#include "engine/player.h"

#define ACCEL 0.1f
#define DECEL 0.1f
#define FRICTION 4

void player_init(player_t *p)
{
	camera_init(&p->cam);
	vector_zero(p->vel, 3);
	p->item_index = -1;
}

static void _player_friction_update(player_t *p)
{
	const float speed = vector_magnitude(p->vel, 3);
	if(speed <= 0.0f) {
		vector_zero(p->vel, 3);
		return;
	}

	float newspeed = speed - (speed * DECEL * FRICTION);
	if(newspeed < 0.001f)
		newspeed = 0;
	newspeed /= speed;
	vector_scale(p->vel, newspeed, 3);
}

static void _player_acceleration_update(player_t *p, update_parms_t uparms)
{
	int cx = uparms.held.c->C_left - uparms.held.c->C_right;
	int cy = uparms.held.c->C_up - uparms.held.c->C_down;
	float forw[3] = {
		cosf(p->cam.yaw) * cy,
		sinf(p->cam.yaw) * cy,
		0
	};
	float side[3] = {
		sinf(-p->cam.yaw) * cx,
		cosf(-p->cam.yaw) * cx,
		0
	};
	float move[3];
	vector_add(forw, side, move, 3);
	vector_normalize(move, 3);
	vector_scale(move, ACCEL, 3);
	vector_add(p->vel, move, p->vel, 3);
}

static void _player_look_update(player_t *p, update_parms_t uparms)
{
	float stick_x = ((float)uparms.held.c->x / 85.0f);
	if(fabsf(stick_x) < 0.1f)
		stick_x = 0;
	float stick_y = ((float)uparms.held.c->y / 85.0f);
	if(fabsf(stick_y) < 0.1f)
		stick_y = 0;

	p->cam.yaw -= stick_x * 0.2f;
	p->cam.pitch -= stick_y * 0.2f;

	p->cam.yaw_smooth = lerpf(p->cam.yaw_smooth, p->cam.yaw, 0.5f);
	if(fabsf(p->cam.yaw_smooth - p->cam.yaw) < 0.01f)
		p->cam.yaw_smooth = p->cam.yaw;

	p->cam.pitch_smooth = lerpf(p->cam.pitch_smooth, p->cam.pitch, 0.5f);
	if(fabsf(p->cam.pitch_smooth - p->cam.pitch) < 0.01f)
		p->cam.pitch_smooth = p->cam.pitch;
}

static void _player_pos_and_focus_update(player_t *p)
{
	vector_copy(p->cam.eye, p->cam.eye_last, 3);
	vector_add(p->cam.eye, p->vel, p->cam.eye, 3);
	vector_copy(p->cam.foc, p->cam.foc_last, 3);
	camera_get_focus(&p->cam);
}

static void _player_pickup_check(player_t *p, scene_t *s)
{
	node_t *n = scene_node_from_name(&s->root_node, "PU.Pistol");
	if(!n) {
		p->item_index = -1;
		return;
	}

	float pistol_pos[3];
	pos_from_mat(n->mat, pistol_pos);
	float pistol_dist[3];
	vector_sub(pistol_pos, p->cam.eye, pistol_dist, 3);

	if(!(int)vector_magnitude(pistol_dist, 3)) {
		n->is_active = false;
		p->item_index = 0;
	}
}

void player_update(player_t *p, scene_t *s, update_parms_t uparms)
{
	_player_friction_update(p);
	_player_acceleration_update(p, uparms);
	_player_look_update(p, uparms);
	_player_pos_and_focus_update(p);
	_player_pickup_check(p, s);
}

void player_view_matrix_setup(const player_t *p, float subtick)
{
	glLoadIdentity();
	float cam_foc_lerp[3];
	float cam_eye_lerp[3];
	vector_lerp(p->cam.foc_last, p->cam.foc, subtick, cam_foc_lerp, 3);
	vector_lerp(p->cam.eye_last, p->cam.eye, subtick, cam_eye_lerp, 3);
	gluLookAt(
		cam_eye_lerp[0], cam_eye_lerp[1], cam_eye_lerp[2],
		cam_foc_lerp[0], cam_foc_lerp[1], cam_foc_lerp[2],
		0, 0, 1);
}
