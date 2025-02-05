#ifndef _ENGINE_COLLISION_H_
#define _ENGINE_COLLISION_H_

#include <stdint.h>

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
} collision_mesh_t;

collision_mesh_t collision_mesh_init_from_file(const char *path);
void collision_mesh_terminate(collision_mesh_t *cm);

#endif /* _ENGINE_COLLISION_H_ */
