#include <GL/gl.h>

#include "engine/util.h"
#include "engine/vector.h"
#include "engine/sfx.h"
#include "engine/player.h"

#define PLAYER_ITEM_TYPE(ITEM_IND) player_item_names[ITEM_IND]

static const char *player_item_names[NUM_ITEM_TYPES] = {
	"PU.Pistol",
	"PU.Bong",
};

void player_item_pickup_check(player_t *p, scene_t *s)
{
	node_t *n;

	for (int i = 0; i < NUM_ITEM_TYPES; i++)
	{
		f32 item_pos[3];
		f32 item_dist[3];

		n = scene_node_from_name(&s->root_node, PLAYER_ITEM_TYPE(i));
		if (!n || !n->is_active)
			continue;
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

void player_item_use_check(player_t *p, struct update_parms uparms)
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

void player_item_swap_check(player_t *p, struct update_parms uparms)
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

void player_items_animation_update(player_t *p)
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
