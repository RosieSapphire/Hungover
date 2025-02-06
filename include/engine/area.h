#ifndef _ENGINE_AREA_H_
#define _ENGINE_AREA_H_

#include <stdio.h>
#include <t3d/t3dmodel.h>

#include "engine/collision.h"
#include "engine/object.h"

typedef struct {
	collision_mesh_t colmesh;
	uint16_t num_objects;
	object_t *objects;
} area_t;

area_t area_read_from_file(FILE *file);
void area_render(const area_t *a, const float subtick);
void area_terminate(area_t *a);

#endif /* _ENGINE_AREA_H_ */
