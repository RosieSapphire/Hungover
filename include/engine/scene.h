#ifndef ENGINE_SCENE_H_
#define ENGINE_SCENE_H_

#include "engine/smesh.h"
#include "engine/animation.h"
#include "engine/node.h"
#include "engine/update.h"

/**
 * enum scene_index - Scene Index
 * @SCENE_TITLE: Title Screen
 * @SCENE_TESTROOM: Test Room
 * @SCENE_COUNT: Numer of Scenes
 *
 * Description: Which scene we are currently interacting with
 */
enum scene_index
{
	SCENE_TITLE,
	SCENE_TESTROOM,
	SCENE_COUNT,
};

extern enum scene_index scene_index;
extern u32 pickup_spin_frame, pickup_spin_frame_last;

typedef struct
{
	uint16_t num_meshes;
	smesh_t *meshes;
	uint16_t num_anims;
	animation_t *anims;
	node_t root_node;
	uint16_t num_tex_indis;
	uint32_t *tex_indis;
} scene_t;

scene_t *scene_load(const char *path);
void scene_unload(scene_t *s);
void scene_update(scene_t *s);
void scene_draw(const scene_t *s, float subtick);
node_t *scene_node_from_name(node_t *n, const char *name);

#endif /* ENGINE_SCENE_H_ */
