#include <stdio.h>
#include <libdragon.h>

#include "engine/collision.h"

// #define DEBUG_COLLISION

collision_mesh_t collision_mesh_init_from_file(const char *path)
{
	collision_mesh_t cm;
	FILE *file = asset_fopen(path, NULL);

	assertf(file, "Failed to load collision mesh from '%s'\n", path);

	fread(&cm.num_triangles, 2, 1, file);
#ifdef DEBUG_COLLISION
	debugf("%d tris\n", cm.num_triangles);
#endif
	cm.triangles = calloc(cm.num_triangles, sizeof *cm.triangles);
	for (uint16_t i = 0; i < cm.num_triangles; i++) {
		collision_triangle_t *f = cm.triangles + i;

#ifdef DEBUG_COLLISION
		debugf("\tface %d\n", i);
#endif
		for (uint16_t j = 0; j < 3; j++) {
			collision_vertex_t *v = f->verts + j;

			for (uint16_t k = 0; k < 3; k++) {
				fread(v->pos + k, 4, 1, file);
			}
#ifdef DEBUG_COLLISION
			debugf("\t\tp%d: (%f, %f, %f)\n", j, v->pos[0],
			       v->pos[1], v->pos[2]);
#endif
		}

		for (uint16_t j = 0; j < 3; j++) {
			fread(f->norm + j, 4, 1, file);
		}
#ifdef DEBUG_COLLISION
		debugf("\t\tnorm: (%f, %f, %f)\n", f->norm[0], f->norm[1],
		       f->norm[2]);
#endif
	}

	fclose(file);

	return cm;
}

void collision_mesh_terminate(collision_mesh_t *cm)
{
	free(cm->triangles);
	cm->triangles = NULL;
	cm->num_triangles = 0;
}
