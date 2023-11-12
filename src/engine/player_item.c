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

/**
 * player_item_pickup_check - Checks for Player Pickup up Item
 * @p: Player Struct
 * @s: Scene with Pickup Item
 */
void player_item_pickup_check(struct player *p, struct scene *s)
{
	struct node *n;

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
		const char *item_paths[NUM_ITEM_TYPES] = {"rom:/pistol.scene",
							  "rom:/bong.scene"};
		const u16 ind = p->num_items - 1;
		const u16 ind2 = p->item_indis[ind] = i;

		p->item_selected = ind;
		p->items[ind].scene = scene_load(item_paths[ind2]);
		p->items[ind].anim_cur = 0;
		struct animation *anim = p->items[ind].scene->anims + 0;

		anim->frame = 0;
		anim->flags &= ~(ANIM_IS_BACKWARD | ANIM_IS_LOOPING);
		anim->flags |= (ANIM_IS_PLAYING);
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

/**
 * player_item_use_check - Checks for Player Using an Item (or Weapon)
 * @p: Player Struct
 * @uparms: Input parameters
 */
void player_item_use_check(struct player *p, struct update_parms uparms)
{
	if (!p->num_items || p->item_selected == NOTHING)
		return;
	struct item *item = p->items + p->item_selected;

	switch (p->item_indis[p->item_selected])
	{
	case PISTOL:
		if (!uparms.down.z || item->cooldown > 0.0f)
			return;

		item->anim_cur = 1;

		struct animation *anim = item->scene->anims + item->anim_cur;

		anim->flags &= ~(ANIM_IS_BACKWARD | ANIM_IS_LOOPING);
		anim->frame = 0;
		item->cooldown = COOLDOWN_PISTOL;
		wav64_play(&fire_pistol, SFXC_ITEM);
		return;
	case BONG:
		if (!uparms.held.z && item->anim_cur == 1)
			item->scene->anims[1].flags |= ANIM_IS_BACKWARD;

		if (uparms.down.z)
		{
			struct animation *anim = item->scene->anims + 1;

			wav64_play(&lighter_bong, SFXC_ITEM);
			item->anim_cur = 1;
			anim->flags &= ~(ANIM_IS_BACKWARD);
			anim->frame = 0;
		}

		item->scene->anims[1].flags &= ~(ANIM_IS_LOOPING);

		return;
	default:
		return;
	}
}

/**
 * player_item_swap_check - Checks for Switching Player's Selected Item
 * @p: Player Struct
 * @uparms: Input parameters
 */
void player_item_swap_check(struct player *p, struct update_parms uparms)
{
	if (p->num_items < 2)
		return;

	if (uparms.down.r)
	{
		p->item_selected = (p->item_selected + 1) % p->num_items;
		struct item *item_cur = p->items + p->item_selected;

		item_cur->anim_cur = 0;
		struct animation *anim = item_cur->scene->anims + 0;

		anim->frame = 0;
		anim->flags &= ~(ANIM_IS_BACKWARD | ANIM_IS_LOOPING);
		anim->flags |= ANIM_IS_PLAYING;
	}
}

/**
 * player_items_animation_update - Updates all Animations for Player Items
 * @p: Player Struct
 */
void player_items_animation_update(struct player *p)
{
	const u16 i = p->item_selected;

	if (i == 0xFFFF)
		return;

	struct item *item = p->items + i;

	scene_update(item->scene);
	if (item->cooldown > 0.0f)
	{
		item->cooldown -= CONF_DELTATIME;
		if (item->cooldown < 0.0f)
			item->cooldown = 0.0f;
	}
}

/**
 * player_item_draw - Draws Player's Currently Held Item
 * @p: Player Struct
 * @subtick: Subtick Between Frames
 */
void player_item_draw(const struct player *p, f32 subtick)
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
	const struct item *item = p->items + ind;
	struct animation *anim = item->scene->anims + item->anim_cur;

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.8f + yaw_turn, -0.75f - pitch_turn, -1.5f);
	glRotatef(-90, 0, 1, 0);
	glRotatef(-90, 1, 0, 0);
	animation_setup_matrix(anim, subtick);
	mesh_draw(item->scene, &p->items[ind].scene->meshes[0]);
	glPopMatrix();
}
