#ifndef _ENGINE_SCENE_H_
#define _ENGINE_SCENE_H_

#include "engine/area.h"

enum {
	SCENE_FLAG_PROCESS_AREA_LAST = (1 << 0),
};

typedef struct {
	uint16_t numAreas;
	Area *areas;
#ifndef IS_USING_SCENE_CONVERTER
	T3DModel *mdl;
	uint16_t areaIndex, areaIndexOld;
	uint8_t flags;
#endif
} Scene;

Scene sceneInitFromFile(const char *path);
void sceneUpdate(Scene *scn, const T3DVec3 *playerPos, const T3DVec3 *playerDir,
		 const float dt);
void sceneRender(const Scene *scn, const float subtick);
void sceneFree(Scene *scn);

#endif /* _ENGINE_SCENE_H_ */
