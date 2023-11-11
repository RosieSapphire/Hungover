#include <GL/gl.h>
#include <GL/glu.h>

#include "engine/vector.h"
#include "engine/util.h"
#include "engine/camera.h"
#include "engine/sfx.h"
#include "engine/player.h"

#define ACCEL 0.1f
#define DECEL 0.1f
#define FRICTION 4

static const char *player_item_names[NUM_ITEM_TYPES] = {
	"PU.Pistol",
	"PU.Bong",
};

#define PLAYER_ITEM_TYPE(ITEM_IND) player_item_names[ITEM_IND]

void player_init(player_t *p)
{
	camera_init(&p->cam);
	vector_zero(p->vel, 3);
	p->item_selected = NOTHING;
	p->num_items = 0;
	memset(p->item_indis, 0, sizeof(u16) * NUM_ITEM_TYPES);
	memset(p->items, 0, sizeof(item_t) * NUM_ITEM_TYPES);
}

static void _player_friction_update(player_t *p)
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

static void _player_acceleration_update(player_t *p, struct update_parms uparms)
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

static void _player_look_update(player_t *p, struct update_parms uparms)
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

static void _player_pos_and_focus_update(player_t *p)
{
	vector_copy(p->cam.eye, p->cam.eye_last, 3);
	vector_add(p->cam.eye, p->vel, p->cam.eye, 3);
	vector_copy(p->cam.foc, p->cam.foc_last, 3);
	camera_get_focus(&p->cam);
}

static void _player_pickup_check(player_t *p, scene_t *s)
{
	node_t *n = NULL;

	for (int i = 0; i < NUM_ITEM_TYPES; i++)
	{
		n = scene_node_from_name(&s->root_node, PLAYER_ITEM_TYPE(i));
		if (!n || !n->is_active)
			continue;

		f32 item_pos[3];
		f32 item_dist[3];

		pos_from_mat(n->mat, item_pos);
		vector_sub(item_pos, p->cam.eye, item_dist, 3);

		if ((int)vector_magnitude_sqr(item_dist, 3))
			continue;

		n->is_active = false;
		p->num_items++;

		const char *item_paths[NUM_ITEM_TYPES] = {
			"rom:/pistol.scene",
			"rom:/bong.scene",
		};

		const u16 ind = p->num_items - 1;
		const u16 ind2 = p->item_indis[ind] = i;

		p->item_selected = ind;
		p->items[ind].scene = scene_load(item_paths[ind2]);
		p->items[ind].anim_cur = 0;
		animation_t *anim = p->items[ind].scene->anims + 0;

		anim->frame = anim->is_backward = anim->loops = 0;
		anim->is_playing = 1;

		switch (ind2)
		{
		case PISTOL:
			wav64_play(&pickup_pistol, SFXC_ITEM);
			break;

		case BONG:
			wav64_play(&pickup_bong, SFXC_ITEM);
			break;
		}
	}
}

static void _player_item_use_check(player_t *p, struct update_parms uparms)
{
	if (!p->num_items || p->item_selected == NOTHING)
		return;

	item_t *item = p->items + p->item_selected;

	switch (p->item_indis[p->item_selected])
	{
	case PISTOL:
		if (!uparms.down.z || item->cooldown > 0.0f)
			return;

		item->anim_cur = 1;

		animation_t *anim = item->scene->anims + item->anim_cur;

		anim->is_backward = anim->loops = 0;
		anim->frame = 0;
		item->cooldown = COOLDOWN_PISTOL;
		wav64_play(&fire_pistol, SFXC_ITEM);
		return;

	case BONG:
		if (!uparms.held.z && item->anim_cur == 1)
			item->scene->anims[1].is_backward = true;

		if (uparms.down.z)
		{
			animation_t *anim = item->scene->anims + 1;

			wav64_play(&lighter_bong, SFXC_ITEM);
			item->anim_cur = 1;
			anim->is_backward = false;
			anim->frame = 0;
		}

		item->scene->anims[1].loops = false;

		return;

	default:
		return;
	}
}

static void _player_item_swap_check(player_t *p, struct update_parms uparms)
{
	if (p->num_items < 2)
		return;

	if (uparms.down.r)
	{
		p->item_selected = (p->item_selected + 1) % p->num_items;
		p->items[p->item_selected].anim_cur = 0;
		animation_t *anim = p->items[p->item_selected].scene->anims + 0;

		anim->frame = anim->loops = anim->is_backward = 0;
		anim->is_playing = 1;
	}
}

static void _player_items_animation_update(player_t *p)
{
	const u16 i = p->item_selected;

	if (i == 0xFFFF)
		return;

	item_t *item = p->items + i;

	scene_update(item->scene);
	if (item->cooldown > 0.0f)
	{
		item->cooldown -= CONF_DELTATIME;
		if (item->cooldown < 0.0f)
			item->cooldown = 0.0f;
	}
}

void player_update(player_t *p, scene_t *s, struct update_parms uparms)
{
	_player_friction_update(p);
	_player_acceleration_update(p, uparms);
	_player_look_update(p, uparms);
	_player_pos_and_focus_update(p);
	_player_pickup_check(p, s);
	_player_item_use_check(p, uparms);
	_player_item_swap_check(p, uparms);
	_player_items_animation_update(p);
}

void player_item_draw(const player_t *p, f32 subtick)
{
	if (p->item_selected == -1 || !p->num_items)
		return;

	static f32 yaw_turn = 0.0f;
	static f32 pitch_turn = 0.0f;
	f32 yaw_dist = p->cam.yaw - p->cam.yaw_last;
	f32 pitch_dist = p->cam.pitch - p->cam.pitch_last;

	if (fabsf(yaw_turn - yaw_dist) < 0.001f)
		yaw_turn = yaw_dist;
	yaw_turn = lerpf(yaw_turn, yaw_dist, 0.2f);

	if (fabsf(pitch_turn - pitch_dist) < 0.001f)
		pitch_turn = pitch_dist;
	pitch_turn = lerpf(pitch_turn, pitch_dist, 0.2f);

	const u16 ind = p->item_selected;
	const item_t *item = p->items + ind;
	animation_t *anim = item->scene->anims + item->anim_cur;

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.8f + yaw_turn, -0.75f - pitch_turn, -1.5f);
	glRotatef(-90, 0, 1, 0);
	glRotatef(-90, 1, 0, 0);
	animation_setup_matrix(anim, subtick);
	smesh_draw(item->scene, &p->items[ind].scene->meshes[0]);
	glPopMatrix();
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
