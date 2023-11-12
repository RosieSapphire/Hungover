#include <GL/gl.h>
#include <GL/glu.h>

#include "engine/vector.h"
#include "engine/util.h"
#include "engine/camera.h"
#include "engine/sfx.h"
#include "engine/player.h"
#include "engine/update.h"

/**
 * player_init - Initializes Player
 * @p: Player in Question
 */
void player_init(struct player *p)
{
	camera_init(&p->cam);
	vector_zero(p->vel, 3);
	p->item_selected = NOTHING;
	p->num_items = 0;
	memset(p->item_indis, 0, sizeof(u16) * NUM_ITEM_TYPES);
	memset(p->items, 0, sizeof(struct item) * NUM_ITEM_TYPES);
}

/**
 * player_update - Updates all Facilities of Player Interaction
 * @p: Player in Question
 * @s: Scene to Interact with
 * @uparms: Input Parameters
 */
void player_update(struct player *p, struct scene *s,
		   struct update_parms uparms)
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

/**
 * player_view_matrix_setup - Sets up OpenGL's View Matrix with Player Camera
 * @p: Player for Camera
 * @subtick: Subtick Between Frames
 */
void player_view_matrix_setup(const struct player *p, f32 subtick)
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
