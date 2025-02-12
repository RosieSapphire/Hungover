#ifndef _ENGINE_COLLISION_H_
#define _ENGINE_COLLISION_H_

#include <stdio.h>
#ifndef IS_USING_SCENE_CONVERTER
#include "types.h"

#include <t3d/t3d.h>
#endif

struct collision_vertex {
	f32 pos[3];
};

struct collision_triangle {
	struct collision_vertex verts[3];
	f32 norm[3];
};

struct collision_mesh {
	u16 triangle_count;
	struct collision_triangle *triangles;
	T3DVec3 offset;
};

struct collision_mesh collision_mesh_init_from_file(FILE *file,
						    const T3DVec3 *offset);
void collision_mesh_free(struct collision_mesh *cm);

#endif /* _ENGINE_COLLISION_H_ */
