#ifndef ENGINE_SCENE_H_
#define ENGINE_SCENE_H_

#include "engine/smesh.h"
#include "engine/animation.h"
#include "engine/node.h"
#include "engine/update.h"

enum scene_index {
	SCENE_TITLE,
	SCENE_TESTROOM,
	SCENE_COUNT,
};

extern enum scene_index scene_index;

typedef struct {
	uint16_t num_meshes;
	smesh_t *meshes;
	uint16_t num_anims;
	animation_t *anims;
	node_t root_node;
} scene_t;

scene_t *scene_load(const char *path);
void scene_unload(scene_t *s);
void scene_update(scene_t *s);
void scene_draw(const scene_t *s, float subtick, const uint32_t tid);
node_t *scene_node_from_name(node_t *n, const char *name);

#endif /* ENGINE_SCENE_H_ */
