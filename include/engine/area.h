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
	collision_mesh_t colmesh;
	uint16_t num_objects;
	object_t *objects;
#ifndef IS_USING_SCENE_CONVERTER
	rspq_block_t *area_block;
	T3DMat4FP *matrix;
#endif
} area_t;

#ifndef IS_USING_SCENE_CONVERTER
area_t area_read_from_file(FILE *file, T3DModel *scene_mdl, const int index);
object_t *area_find_door_by_dest_index(const area_t *a,
				       const uint16_t dest_index);
void area_render(const area_t *a, const float subtick);
void area_terminate(area_t *a);
#endif

#endif /* _ENGINE_AREA_H_ */
