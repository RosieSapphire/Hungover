#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "util.h"

/* C89 doesn't have this declared by default. :/ */
extern float sqrtf(float);

void exitf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	exit(EXIT_FAILURE);
}

void triangle_calc_normal(struct collision_triangle *tri)
{
	float a[3], b[3], mag;

	a[0] = tri->verts[1].pos[0] - tri->verts[0].pos[0];
	a[1] = tri->verts[1].pos[1] - tri->verts[0].pos[1];
	a[2] = tri->verts[1].pos[2] - tri->verts[0].pos[2];

	b[0] = tri->verts[2].pos[0] - tri->verts[0].pos[0];
	b[1] = tri->verts[2].pos[1] - tri->verts[0].pos[1];
	b[2] = tri->verts[2].pos[2] - tri->verts[0].pos[2];

	tri->norm[0] = a[1] * b[2] - a[2] * b[1];
	tri->norm[1] = a[2] * b[0] - a[0] * b[2];
	tri->norm[2] = a[0] * b[1] - a[1] * b[0];

	mag = sqrtf(tri->norm[0] * tri->norm[0] + tri->norm[1] * tri->norm[1] +
		    tri->norm[2] * tri->norm[2]);

	if (!mag) {
		return;
	}

	tri->norm[0] /= mag;
	tri->norm[1] /= mag;
	tri->norm[2] /= mag;
}

void quaternion_from_matrix(float quat[4], float matrix[4][4])
{
	float r, rinv;
	float trace = matrix[0][0] + matrix[1][1] + matrix[2][2];

	if (trace >= 0.0f) {
		r = sqrtf(1.0f + trace);
		rinv = 0.5f / r;

		quat[0] = rinv * (matrix[1][2] - matrix[2][1]);
		quat[1] = rinv * (matrix[2][0] - matrix[0][2]);
		quat[2] = rinv * (matrix[0][1] - matrix[1][0]);
		quat[3] = r * 0.5f;
	} else if (matrix[0][0] >= matrix[1][1] &&
		   matrix[0][0] >= matrix[2][2]) {
		r = sqrtf(1.0f - matrix[1][1] - matrix[2][2] + matrix[0][0]);
		rinv = 0.5f / r;

		quat[0] = r * 0.5f;
		quat[1] = rinv * (matrix[0][1] + matrix[1][0]);
		quat[2] = rinv * (matrix[0][2] + matrix[2][0]);
		quat[3] = rinv * (matrix[1][2] - matrix[2][1]);
	} else if (matrix[1][1] >= matrix[2][2]) {
		r = sqrtf(1.0f - matrix[0][0] - matrix[2][2] + matrix[1][1]);
		rinv = 0.5f / r;

		quat[0] = rinv * (matrix[0][1] + matrix[1][0]);
		quat[1] = r * 0.5f;
		quat[2] = rinv * (matrix[1][2] + matrix[2][1]);
		quat[3] = rinv * (matrix[2][0] - matrix[0][2]);
	} else {
		r = sqrtf(1.0f - matrix[0][0] - matrix[1][1] + matrix[2][2]);
		rinv = 0.5f / r;

		quat[0] = rinv * (matrix[0][2] + matrix[2][0]);
		quat[1] = rinv * (matrix[1][2] + matrix[2][1]);
		quat[2] = r * 0.5f;
		quat[3] = rinv * (matrix[0][1] - matrix[1][0]);
	}
}

void scene_debug(const struct scene *s, const char *scene_path)
{
	u16 i, j, k;

#ifndef GLB_TO_SCN_DEBUG
	return;
#endif /* GLB_TO_SCN_DEBUG */

	printf("Scene '%s' (%d areas)\n", scene_path, s->area_count);
	for (i = 0; i < s->area_count; i++) {
		struct area *a = s->areas + i;

		printf("\tArea %d (%d Actors):\n", i, a->actor_header_count);
		printf("\t\tOffset: (%f, %f, %f)\n", a->offset.v[0],
		       a->offset.v[1], a->offset.v[2]);
		printf("\t\tCollision Mesh (%d triangles)\n",
		       a->colmesh.triangle_count);
		printf("\t\t\tOffset: (%f, %f, %f)\n", a->colmesh.offset.v[0],
		       a->colmesh.offset.v[1], a->colmesh.offset.v[2]);
		for (j = 0; j < a->colmesh.triangle_count; j++) {
			struct collision_triangle *tri =
				a->colmesh.triangles + j;

			printf("\t\t\tTri %d:\n", j);
			for (k = 0; k < 3; k++) {
				printf("\t\t\t\tPos %d: (%f, %f, %f)\n", k,
				       tri->verts[k].pos[0],
				       tri->verts[k].pos[1],
				       tri->verts[k].pos[2]);
			}
			printf("\t\t\t\tNorm: (%f, %f, %f)\n", tri->norm[0],
			       tri->norm[1], tri->norm[2]);
		}

		for (j = 0; j < a->actor_header_count; j++) {
			struct actor_header *actor = a->actor_headers + j;

			printf("\t\tActor %d ('%s'):\n", j, actor->name);
			printf("\t\t\tPosition: (%f, %f, %f):\n",
			       actor->position.v[0], actor->position.v[1],
			       actor->position.v[2]);
			printf("\t\t\tRotation: (%f, %f, %f, %f):\n",
			       actor->rotation.v[0], actor->rotation.v[1],
			       actor->rotation.v[2], actor->rotation.v[3]);
			printf("\t\t\tScale: (%f, %f, %f):\n",
			       actor->scale.v[0], actor->scale.v[1],
			       actor->scale.v[2]);
		}
	}
}
