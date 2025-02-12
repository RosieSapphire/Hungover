#include <libdragon.h>
#include <stdio.h>

#include "engine/collision.h"

// #define DEBUG_COLLISION

struct collision_mesh collision_mesh_init_from_file(FILE *file,
						    const T3DVec3 *offset)
{
	struct collision_mesh cm;

	fread(&cm.triangle_count, 2, 1, file);
#ifdef DEBUG_COLLISION
	debugf("%d tris\n", cm.triangle_count);
#endif
	cm.triangles = calloc(cm.triangle_count, sizeof(*cm.triangles));
	for (u16 i = 0; i < cm.triangle_count; i++) {
		struct collision_triangle *f = cm.triangles + i;

#ifdef DEBUG_COLLISION
		debugf("\tface %d\n", i);
#endif
		for (u16 j = 0; j < 3; j++) {
			struct collision_vertex *v = f->verts + j;

			for (u16 k = 0; k < 3; k++) {
				fread(v->pos + k, 4, 1, file);
			}
#ifdef DEBUG_COLLISION
			debugf("\t\tp%d: (%f, %f, %f)\n", j, v->pos[0],
			       v->pos[1], v->pos[2]);
#endif
		}

		for (u16 j = 0; j < 3; j++) {
			fread(f->norm + j, 4, 1, file);
		}
#ifdef DEBUG_COLLISION
		debugf("\t\tnorm: (%f, %f, %f)\n", f->norm[0], f->norm[1],
		       f->norm[2]);
#endif
	}

	for (u16 i = 0; i < 3; i++) {
		fread(cm.offset.v + i, 4, 1, file);
		cm.offset.v[i] += offset->v[i];
	}

	return cm;
}

void collision_mesh_free(struct collision_mesh *cm)
{
	free(cm->triangles);
	cm->triangles = NULL;
	cm->triangle_count = 0;
}
