#ifndef ENGINE_PLAYER_H_
#define ENGINE_PLAYER_H_

#include "engine/update.h"
#include "engine/camera.h"
#include "engine/scene.h"

#define COOLDOWN_PISTOL 0.21f
#define COOLDOWN_BONG 2.0f

enum item_index
{
	NOTHING = -1,
	PISTOL,
	BONG,
	NUM_ITEM_TYPES,
};

typedef struct
{
	scene_t *scene;
	int16_t anim_cur;
	float cooldown;
} item_t;

typedef struct
{
	camera_t cam;
	float vel[3];
	enum item_index item_selected;
	uint16_t num_items;
	uint16_t item_indis[NUM_ITEM_TYPES];
	item_t items[NUM_ITEM_TYPES];
} player_t;

/*
 * Main
 */
void player_init(player_t *p);
void player_update(player_t *p, scene_t *s, struct update_parms uparms);
void player_view_matrix_setup(const player_t *p, float subtick);
void player_item_draw(const player_t *p, float subtick);

/*
 * Items
 */
void player_item_pickup_check(player_t *p, scene_t *s);
void player_item_use_check(player_t *p, struct update_parms uparms);
void player_item_swap_check(player_t *p, struct update_parms uparms);
void player_items_animation_update(player_t *p);
void player_item_draw(const player_t *p, f32 subtick);

/*
 * Input
 */
void player_friction_update(player_t *p);
void player_acceleration_update(player_t *p, struct update_parms uparms);
void player_look_update(player_t *p, struct update_parms uparms);
void player_pos_and_focus_update(player_t *p);

#endif /* ENGINE_PLAYER_H_ */
