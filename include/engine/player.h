#ifndef ENGINE_PLAYER_H_
#define ENGINE_PLAYER_H_

#include "engine/update.h"
#include "engine/camera.h"
#include "engine/scene.h"

#define COOLDOWN_PISTOL 0.21f
#define COOLDOWN_BONG 2.0f

/**
 * enum item_index - Index for Player Items
 * @NOTHING: Nothing... dumbass
 * @PISTOL: Pistol... dumbass
 * @BONG: I'm done writing descriptions for these
 * @NUM_ITEM_TYPES: Number of Item Types, dipshit
 */
enum item_index
{
	NOTHING = -1,
	PISTOL,
	BONG,
	NUM_ITEM_TYPES,
};

/**
 * struct item - Item Structure
 * @scene: Scene for Item
 * @anim_cur: Current Animation
 * @cooldown: Cooldown Timer
 */
struct item
{
	struct scene *scene;
	s16 anim_cur;
	f32 cooldown;
};

/**
 * struct player - Player Structure
 * @cam: Attached Camera
 * @vel: Velocity Vector
 * @item_selected: Current Item Selected
 * @num_items: Number of Items
 * @item_indis: Item Indices
 * @items: Items Array
 */
struct player
{
	struct camera cam;
	f32 vel[3];
	enum item_index item_selected;
	u16 num_items;
	u16 item_indis[NUM_ITEM_TYPES];
	struct item items[NUM_ITEM_TYPES];
};

/*
 * Main
 */
void player_init(struct player *p);
void player_update(struct player *p, struct scene *s,
		   struct update_parms uparms);
void player_view_matrix_setup(const struct player *p, f32 subtick);
void player_item_draw(const struct player *p, f32 subtick);

/*
 * Items
 */
void player_item_pickup_check(struct player *p, struct scene *s);
void player_item_use_check(struct player *p, struct update_parms uparms);
void player_item_swap_check(struct player *p, struct update_parms uparms);
void player_items_animation_update(struct player *p);
void player_item_draw(const struct player *p, f32 subtick);

/*
 * Input
 */
void player_friction_update(struct player *p);
void player_acceleration_update(struct player *p, struct update_parms uparms);
void player_look_update(struct player *p, struct update_parms uparms);
void player_pos_and_focus_update(struct player *p);

#endif /* ENGINE_PLAYER_H_ */
