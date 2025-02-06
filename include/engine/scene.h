#ifndef _ENGINE_SCENE_H_
#define _ENGINE_SCENE_H_

#include "engine/area.h"

typedef struct {
	uint16_t num_areas;
	area_t *areas;
#ifndef IS_USING_SCENE_CONVERTER
	T3DModel *mdl;
	uint16_t area_index;
#endif
} scene_t;

scene_t scene_init_from_file(const char *path);
void scene_render(const scene_t *scn, const float subtick);
void scene_terminate(scene_t *scn);

#endif /* _ENGINE_SCENE_H_ */
