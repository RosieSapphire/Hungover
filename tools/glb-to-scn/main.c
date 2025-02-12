#define IS_USING_SCENE_CONVERTER

#define T3D_RAD_TO_DEG(deg) (deg * 57.29577951289617186798f)

// #define GLB_TO_SCN_DEBUG

typedef struct {
	float v[3];
} T3DVec3;

typedef struct {
	float v[4];
} T3DQuat;

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include "config.h"
#include "engine/scene.h"

static void exitf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	exit(EXIT_FAILURE);
}

static void triangle_calc_normal(struct collision_triangle *tri)
{
	float a[3] = {
		tri->verts[1].pos[0] - tri->verts[0].pos[0],
		tri->verts[1].pos[1] - tri->verts[0].pos[1],
		tri->verts[1].pos[2] - tri->verts[0].pos[2],
	};

	float b[3] = {
		tri->verts[2].pos[0] - tri->verts[0].pos[0],
		tri->verts[2].pos[1] - tri->verts[0].pos[1],
		tri->verts[2].pos[2] - tri->verts[0].pos[2],
	};

	tri->norm[0] = a[1] * b[2] - a[2] * b[1];
	tri->norm[1] = a[2] * b[0] - a[0] * b[2];
	tri->norm[2] = a[0] * b[1] - a[1] * b[0];

	float mag = sqrtf(tri->norm[0] * tri->norm[0] +
			  tri->norm[1] * tri->norm[1] +
			  tri->norm[2] * tri->norm[2]);

	if (!mag) {
		return;
	}

	tri->norm[0] /= mag;
	tri->norm[1] /= mag;
	tri->norm[2] /= mag;
}

static struct collision_mesh
collision_mesh_from_assimp(const struct aiScene *aiscn,
			   const struct aiNode *ainode)
{
	struct collision_mesh ret;

	u16 numMeshes = ainode->mNumMeshes;
	struct collision_mesh *meshesOut = calloc(numMeshes, sizeof *meshesOut);
	for (int i = 0; i < numMeshes; i++) {
		const struct aiMesh *in = aiscn->mMeshes[ainode->mMeshes[i]];
		struct collision_mesh *out = meshesOut + i;

		out->triangle_count = in->mNumFaces;
		out->triangles =
			calloc(out->triangle_count, sizeof *out->triangles);
		for (u16 i = 0; i < out->triangle_count; i++) {
			const struct aiFace *tri_in = in->mFaces + i;
			struct collision_triangle *triOut = out->triangles + i;

			for (u16 j = 0; j < 3; j++) {
				struct collision_vertex *vert_out =
					triOut->verts + j;
				const struct aiVector3D vert_in =
					in->mVertices[tri_in->mIndices[j]];

				vert_out->pos[0] =
					vert_in.x * T3DM_TO_N64_SCALE;
				vert_out->pos[1] =
					vert_in.y * T3DM_TO_N64_SCALE;
				vert_out->pos[2] =
					vert_in.z * T3DM_TO_N64_SCALE;
			}
			triangle_calc_normal(triOut);
		}
	}

	ret.triangle_count = 0;
	ret.triangles = malloc(0);
	for (u16 i = 0; i < numMeshes; i++) {
		const struct collision_mesh *m = meshesOut + i;

		ret.triangle_count += m->triangle_count;
		ret.triangles =
			realloc(ret.triangles,
				sizeof *ret.triangles * ret.triangle_count);
		memcpy(ret.triangles + (ret.triangle_count - m->triangle_count),
		       m->triangles, sizeof *m->triangles * m->triangle_count);
	}
	ret.offset.v[0] = ainode->mTransformation.a4 * T3DM_TO_N64_SCALE;
	ret.offset.v[1] = ainode->mTransformation.b4 * T3DM_TO_N64_SCALE;
	ret.offset.v[2] = ainode->mTransformation.c4 * T3DM_TO_N64_SCALE;

	return ret;
}

static unsigned long fwrite_ef16(const u16 *ptr, FILE *file)
{
	u16 flip = ((*ptr & 0x00FF) << 8) | ((*ptr & 0xFF00) >> 8);

	return fwrite(&flip, 2, 1, file);
}

static unsigned long fwrite_ef32(const float *ptr, FILE *file)
{
	uint32_t bytes = *((uint32_t *)ptr);

	bytes = ((bytes & 0x000000FF) << 24) | ((bytes & 0x0000FF00) << 8) |
		((bytes & 0x00FF0000) >> 8) | ((bytes & 0xFF000000) >> 24);

	return fwrite(((float *)&bytes), 4, 1, file);
}

#ifdef RUN_TEST
static unsigned long freadEF16(u16 *ptr, FILE *file)
{
	unsigned long ret = fread(ptr, 2, 1, file);
	u16 bytes = *((u16 *)ptr);

	bytes = ((bytes & 0x00FF) << 8) | ((bytes & 0xFF00) >> 8);
	*ptr = *((u16 *)&bytes);

	return ret;
}

static unsigned long freadEF32(float *ptr, FILE *file)
{
	unsigned long ret = fread(ptr, sizeof *ptr, 1, file);
	uint32_t bytes = *((uint32_t *)ptr);

	bytes = ((bytes & 0x000000FF) << 24) | ((bytes & 0x0000FF00) << 8) |
		((bytes & 0x00FF0000) >> 8) | ((bytes & 0xFF000000) >> 24);
	*ptr = *((float *)&bytes);

	return ret;
}
#endif

static void collision_mesh_write_to_file(const struct collision_mesh *cm,
					 FILE *file)
{
	fwrite_ef16(&cm->triangle_count, file);
	for (u16 i = 0; i < cm->triangle_count; i++) {
		struct collision_triangle *tri = cm->triangles + i;

		for (u16 j = 0; j < 3; j++) {
			for (u16 k = 0; k < 3; k++) {
				fwrite_ef32(tri->verts[j].pos + k, file);
			}
		}

		for (u16 j = 0; j < 3; j++) {
			fwrite_ef32(tri->norm + j, file);
		}
	}

	for (u16 i = 0; i < 3; i++) {
		fwrite_ef32(cm->offset.v + i, file);
	}
}

static void quaternionFromMatrix(float quat[4], const float matrix[4][4])
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

static void actor_write_to_file(const struct actor_header *actor, FILE *file)
{
	fwrite(actor->name, 1, ACTOR_NAME_MAX_LEN, file);

	for (int i = 0; i < 3; i++) {
		fwrite_ef32(actor->position.v + i, file);
	}

	for (int i = 0; i < 4; i++) {
		fwrite_ef32(actor->rotation.v + i, file);
	}

	for (int i = 0; i < 3; i++) {
		fwrite_ef32(actor->scale.v + i, file);
	}
}

static void area_write_to_file(const struct area *a, FILE *file)
{
	for (int i = 0; i < 3; i++) {
		fwrite_ef32(a->offset.v + i, file);
	}
	collision_mesh_write_to_file(&a->colmesh, file);
	fwrite_ef16(&a->actor_header_count, file);
	for (u16 i = 0; i < a->actor_header_count; i++) {
		actor_write_to_file(a->actor_headers + i, file);
	}
}

static void scene_write_to_file(const struct scene *scn, FILE *file)
{
	fwrite_ef16(&scn->area_count, file);
	for (u16 i = 0; i < scn->area_count; i++) {
		area_write_to_file(scn->areas + i, file);
	}
#ifdef GLB_TO_SCN_DEBUG
	printf("Successfully wrote scene to file\n");
#endif /* GLB_TO_SCN_DEBUG */
}

static void scene_area_process(struct area *a, const struct aiScene *aiscn,
			       const struct aiNode *n)
{
	a->actor_header_count = 0;
	a->actor_headers = malloc(0);
	a->offset.v[0] = n->mTransformation.a4 * T3DM_TO_N64_SCALE;
	a->offset.v[1] = n->mTransformation.b4 * T3DM_TO_N64_SCALE;
	a->offset.v[2] = n->mTransformation.c4 * T3DM_TO_N64_SCALE;
	int numColMeshes = 0;
	for (unsigned int i = 0; i < n->mNumChildren; i++) {
		const struct aiNode *ch = n->mChildren[i];

		if (!strncmp(ch->mName.data, "Col", 3)) {
			a->colmesh = collision_mesh_from_assimp(aiscn, ch);
			numColMeshes++;
			if (numColMeshes > 1) {
				exitf("ERROR: struct area can only have "
				      "one collision mesh\n");
			}
		} else if (!strncmp(ch->mName.data, "Act", 3)) {
			a->actor_headers =
				realloc(a->actor_headers,
					sizeof(*a->actor_headers) *
						++a->actor_header_count);
			struct actor_header *onew =
				a->actor_headers + a->actor_header_count - 1;
			strncpy(onew->name, ch->mName.data, ACTOR_NAME_MAX_LEN);
			onew->position.v[0] =
				ch->mTransformation.a4 * T3DM_TO_N64_SCALE;
			onew->position.v[1] =
				ch->mTransformation.b4 * T3DM_TO_N64_SCALE;
			onew->position.v[2] =
				ch->mTransformation.c4 * T3DM_TO_N64_SCALE;
			const float matrix[4][4] = {
				{ ch->mTransformation.a1,
				  ch->mTransformation.b1,
				  ch->mTransformation.c1,
				  ch->mTransformation.d1 },

				{ ch->mTransformation.a2,
				  ch->mTransformation.b2,
				  ch->mTransformation.c2,
				  ch->mTransformation.d2 },

				{ ch->mTransformation.a3,
				  ch->mTransformation.b3,
				  ch->mTransformation.c3,
				  ch->mTransformation.d3 },

				{ ch->mTransformation.a4,
				  ch->mTransformation.b4,
				  ch->mTransformation.c4,
				  ch->mTransformation.d4 },
			};
			quaternionFromMatrix(onew->rotation.v, matrix);
			onew->scale = (T3DVec3){ {
				ch->mTransformation.a1,
				ch->mTransformation.b2,
				ch->mTransformation.c3,
			} };
		} else {
			exitf("ERROR: Invalid Item Type.\n");
		}
	}
}

static void assimp_scene_node_process(struct scene *scn,
				      const struct aiScene *aiscn,
				      const struct aiNode *n)
{
	if (!strncmp("Area", n->mName.data, 4)) {
		scn->areas = realloc(scn->areas,
				     sizeof *scn->areas * ++scn->area_count);
		scene_area_process(scn->areas + scn->area_count - 1, aiscn, n);
	}

	for (unsigned int i = 0; i < n->mNumChildren; i++) {
		assimp_scene_node_process(scn, aiscn, n->mChildren[i]);
	}
}

static void scene_debug(const struct scene *s, const char *scene_path)
{
#ifndef GLB_TO_SCN_DEBUG
	return;
#endif /* GLB_TO_SCN_DEBUG */

	printf("DEBUGGING SCENE '%s' (%d areas)\n", scene_path, s->area_count);
	for (u16 i = 0; i < s->area_count; i++) {
		struct area *a = s->areas + i;

		printf("\tstruct area %d (%d actor_headers):\n", i,
		       a->actor_header_count);
		printf("\t\tOffset: (%f, %f, %f)\n", a->offset.v[0],
		       a->offset.v[1], a->offset.v[2]);
		printf("\t\tCollision Mesh (%d triangles)\n",
		       a->colmesh.triangle_count);
		printf("\t\t\tOffset: (%f, %f, %f)\n", a->colmesh.offset.v[0],
		       a->colmesh.offset.v[1], a->colmesh.offset.v[2]);
		for (u16 j = 0; j < a->colmesh.triangle_count; j++) {
			struct collision_triangle *tri =
				a->colmesh.triangles + j;

			printf("\t\t\tTri %d:\n", j);
			for (u16 k = 0; k < 3; k++) {
				printf("\t\t\t\tPos %d: (%f, %f, %f)\n", k,
				       tri->verts[k].pos[0],
				       tri->verts[k].pos[1],
				       tri->verts[k].pos[2]);
			}
			printf("\t\t\t\tNorm: (%f, %f, %f)\n", tri->norm[0],
			       tri->norm[1], tri->norm[2]);
		}

		for (u16 j = 0; j < a->actor_header_count; j++) {
			struct actor_header *actor = a->actor_headers + j;

			printf("\t\tstruct actor_header %d ('%s'):\n", j,
			       actor->name);
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

static struct scene scene_handle(const struct aiScene *aiscn,
				 const char *scene_path)
{
	struct scene scn;
	scn.area_count = 0;
	scn.areas = malloc(0);

	assimp_scene_node_process(&scn, aiscn, aiscn->mRootNode);
	scene_debug(&scn, scene_path);

	FILE *scene_file = fopen(scene_path, "wb");

	if (!scene_file) {
		exitf("Failed to write scene to '%s'\n", scene_path);
	}

	scene_write_to_file(&scn, scene_file);

	fclose(scene_file);

	return scn;
}

int main(int argc, char **argv)
{
	char scene_path[512];
	const char *glb_path = argv[1];

	if (strncmp(strrchr(glb_path, '/') + 1, "Scn.", 4)) {
		/* This is not a scene and we can safely skip it */
		return 0;
	}

	memset(scene_path, 0, 512);

	switch (argc) {
	default:
		exitf("Invalid number of arguments.\n");
		return 1;

	case 2:
		snprintf(scene_path, strlen(glb_path) - 3, "%s", glb_path);
		snprintf(scene_path + strlen(scene_path), 5, ".col");
		break;

	case 3:
		strncpy(scene_path, argv[2], 512);
		break;
	}

	const struct aiScene *scene =
		aiImportFile(glb_path, aiProcess_Triangulate |
					       aiProcess_JoinIdenticalVertices);

	if (!scene) {
		exitf("Failed to load scene from '%s'\n", glb_path);
	}

	scene_handle(scene, scene_path);

	return 0;
}
