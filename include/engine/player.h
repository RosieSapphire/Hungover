#ifndef ENGINE_PLAYER_H_
#define ENGINE_PLAYER_H_

#include "engine/update.h"
#include "engine/camera.h"
#include "engine/scene.h"

enum item_index {
	NOTHING = -1,
	PISTOL,
	BONG,
	NUM_ITEM_TYPES,
};

typedef struct {
	smesh_t mesh;
	uint16_t num_anims;
	animation_t *anims;
	int16_t anim_cur;
} item_t;

typedef struct {
	camera_t cam;
	float vel[3];
	enum item_index item_index;
	uint16_t num_items;
	item_t *items;
} player_t;

void player_init(player_t *p);
void player_update(player_t *p, scene_t *s, update_parms_t uparms);
void player_view_matrix_setup(const player_t *p, float subtick);
void player_item_draw(const player_t *p, const uint32_t tid);

#endif /* ENGINE_PLAYER_H_ */
