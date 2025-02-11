#include <stdio.h>
#include <libdragon.h>

#include "engine/collision.h"

// #define DEBUG_COLLISION

CollisionMesh collisionMeshInitFromFile(FILE *file, const T3DVec3 *offset)
{
	CollisionMesh cm;

	fread(&cm.numTriangles, 2, 1, file);
#ifdef DEBUG_COLLISION
	debugf("%d tris\n", cm.numTriangles);
#endif
	cm.triangles = calloc(cm.numTriangles, sizeof *cm.triangles);
	for (uint16_t i = 0; i < cm.numTriangles; i++) {
		CollisionTriangle *f = cm.triangles + i;

#ifdef DEBUG_COLLISION
		debugf("\tface %d\n", i);
#endif
		for (uint16_t j = 0; j < 3; j++) {
			CollisionVertex *v = f->verts + j;

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

	for (uint16_t i = 0; i < 3; i++) {
		fread(cm.offset.v + i, 4, 1, file);
		cm.offset.v[i] += offset->v[i];
	}

	return cm;
}

void collisionMeshFree(CollisionMesh *cm)
{
	free(cm->triangles);
	cm->triangles = NULL;
	cm->numTriangles = 0;
}
