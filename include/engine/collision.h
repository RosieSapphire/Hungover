#ifndef _ENGINE_COLLISION_H_
#define _ENGINE_COLLISION_H_

#include <stdio.h>
#include <stdint.h>
#ifndef IS_USING_SCENE_CONVERTER
#include <t3d/t3d.h>
#endif

typedef struct {
	float pos[3];
} collision_vertex_t;

typedef struct {
	collision_vertex_t verts[3];
	float norm[3];
} collision_triangle_t;

typedef struct {
	uint16_t num_triangles;
	collision_triangle_t *triangles;
	T3DVec3 offset;
} collision_mesh_t;

collision_mesh_t collision_mesh_read_from_file(FILE *file,
					       const T3DVec3 *offset);
void collision_mesh_terminate(collision_mesh_t *cm);

#endif /* _ENGINE_COLLISION_H_ */
