#ifndef _ENGINE_AREA_H_
#define _ENGINE_AREA_H_

#include <stdio.h>

#ifndef IS_USING_SCENE_CONVERTER
#include <t3d/t3dmodel.h>
#endif

#include "engine/collision.h"
#include "engine/object.h"

typedef struct {
	T3DVec3 offset;
	CollisionMesh colmesh;
	uint16_t numObjects;
	Object *objects;
#ifndef IS_USING_SCENE_CONVERTER
	rspq_block_t *areaBlock;
	T3DMat4FP *matrix;
#endif
} Area;

#ifndef IS_USING_SCENE_CONVERTER
Area areaInitFromFile(FILE *file, T3DModel *sceneModel, const int index);
Object *areaFindDoorFromDestIndex(const Area *a, const uint16_t destIndex);
void areaRender(const Area *a, const float subtick);
void areaFree(Area *a);
#endif

#endif /* _ENGINE_AREA_H_ */
