#include <GL/gl.h>
#include <GL/glu.h>

#include "engine/vector.h"
#include "engine/util.h"
#include "engine/camera.h"
#include "engine/sfx.h"
#include "engine/player.h"

void player_init(player_t *p)
{
	camera_init(&p->cam);
	vector_zero(p->vel, 3);
	p->item_selected = NOTHING;
	p->num_items = 0;
	memset(p->item_indis, 0, sizeof(u16) * NUM_ITEM_TYPES);
	memset(p->items, 0, sizeof(item_t) * NUM_ITEM_TYPES);
}

void player_update(player_t *p, scene_t *s, struct update_parms uparms)
{
	player_friction_update(p);
	player_acceleration_update(p, uparms);
	player_look_update(p, uparms);
	player_pos_and_focus_update(p);
	player_item_pickup_check(p, s);
	player_item_use_check(p, uparms);
	player_item_swap_check(p, uparms);
	player_items_animation_update(p);
}

void player_view_matrix_setup(const player_t *p, f32 subtick)
{
	f32 cam_foc_lerp[3];
	f32 cam_eye_lerp[3];

	glLoadIdentity();
	vector_lerp(p->cam.foc_last, p->cam.foc, subtick, cam_foc_lerp, 3);
	vector_lerp(p->cam.eye_last, p->cam.eye, subtick, cam_eye_lerp, 3);
	gluLookAt(
		cam_eye_lerp[0], cam_eye_lerp[1], cam_eye_lerp[2],
		cam_foc_lerp[0], cam_foc_lerp[1], cam_foc_lerp[2],
		0, 0, 1);
}
