#ifndef ENGINE_SCENE_H_
#define ENGINE_SCENE_H_

#include "engine/smesh.h"

enum scene_index {
	SCENE_TITLE,
	SCENE_TESTROOM,
	SCENE_COUNT,
};

extern enum scene_index scene_index;

typedef struct {
	uint16_t num_meshes;
	smesh_t *meshes;
} scene_t;

scene_t *scene_load(const char *path);
void scene_unload(scene_t *s);
void scene_draw(const scene_t *s, /* temp */const uint32_t tid);

#endif /* ENGINE_SCENE_H_ */
