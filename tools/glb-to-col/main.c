#define IS_USING_SCENE_CONVERTER

typedef struct {
	float v[3];
} T3DVec3;

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

static void triangle_calc_normal(collision_triangle_t *tri)
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

static collision_mesh_t collision_mesh_from_assimp(const struct aiScene *aiscn,
						   const struct aiNode *ainode)
{
	collision_mesh_t ret;

	uint16_t num_meshes = ainode->mNumMeshes;
	collision_mesh_t *meshes_out = calloc(num_meshes, sizeof *meshes_out);
	for (int i = 0; i < num_meshes; i++) {
		const struct aiMesh *in = aiscn->mMeshes[ainode->mMeshes[i]];
		collision_mesh_t *out = meshes_out + i;

		out->num_triangles = in->mNumFaces;
		out->triangles =
			calloc(out->num_triangles, sizeof *out->triangles);
		for (uint16_t i = 0; i < out->num_triangles; i++) {
			const struct aiFace *tri_in = in->mFaces + i;
			collision_triangle_t *tri_out = out->triangles + i;

			for (uint16_t j = 0; j < 3; j++) {
				collision_vertex_t *vert_out =
					tri_out->verts + j;
				const struct aiVector3D vert_in =
					in->mVertices[tri_in->mIndices[j]];

				vert_out->pos[0] =
					vert_in.x * T3DM_TO_N64_SCALE;
				vert_out->pos[1] =
					vert_in.y * T3DM_TO_N64_SCALE;
				vert_out->pos[2] =
					vert_in.z * T3DM_TO_N64_SCALE;
			}
			triangle_calc_normal(tri_out);
		}
	}

	ret.num_triangles = 0;
	ret.triangles = malloc(0);
	for (uint16_t i = 0; i < num_meshes; i++) {
		const collision_mesh_t *m = meshes_out + i;

		ret.num_triangles += m->num_triangles;
		ret.triangles =
			realloc(ret.triangles,
				sizeof *ret.triangles * ret.num_triangles);
		memcpy(ret.triangles + (ret.num_triangles - m->num_triangles),
		       m->triangles, sizeof *m->triangles * m->num_triangles);
	}
	ret.offset.v[0] = ainode->mTransformation.a4;
	ret.offset.v[1] = ainode->mTransformation.b4;
	ret.offset.v[2] = ainode->mTransformation.c4;

	return ret;
}

static unsigned long fwrite_ef16(const uint16_t *ptr, FILE *file)
{
	uint16_t flip = ((*ptr & 0x00FF) << 8) | ((*ptr & 0xFF00) >> 8);

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
static unsigned long fread_ef16(uint16_t *ptr, FILE *file)
{
	unsigned long ret = fread(ptr, 2, 1, file);
	uint16_t bytes = *((uint16_t *)ptr);

	bytes = ((bytes & 0x00FF) << 8) | ((bytes & 0xFF00) >> 8);
	*ptr = *((uint16_t *)&bytes);

	return ret;
}

static unsigned long fread_ef32(float *ptr, FILE *file)
{
	unsigned long ret = fread(ptr, sizeof *ptr, 1, file);
	uint32_t bytes = *((uint32_t *)ptr);

	bytes = ((bytes & 0x000000FF) << 24) | ((bytes & 0x0000FF00) << 8) |
		((bytes & 0x00FF0000) >> 8) | ((bytes & 0xFF000000) >> 24);
	*ptr = *((float *)&bytes);

	return ret;
}
#endif

static void collision_mesh_write_to_file(const collision_mesh_t *cm, FILE *file)
{
	fwrite_ef16(&cm->num_triangles, file);
	for (uint16_t i = 0; i < cm->num_triangles; i++) {
		collision_triangle_t *tri = cm->triangles + i;

		for (uint16_t j = 0; j < 3; j++) {
			for (uint16_t k = 0; k < 3; k++) {
				fwrite_ef32(tri->verts[j].pos + k, file);
			}
		}

		for (uint16_t j = 0; j < 3; j++) {
			fwrite_ef32(tri->norm + j, file);
		}
	}

	T3DVec3 offset = cm->offset;

	for (uint16_t i = 0; i < 3; i++) {
		offset.v[i] *= T3DM_TO_N64_SCALE;
		fwrite_ef32(offset.v + i, file);
	}
}

static void object_write_to_file(const object_t *o, FILE *file)
{
	fwrite(o->name, 1, OBJECT_NAME_MAX_LENGTH, file);

	T3DVec3 pos_out = o->position;

	for (int i = 0; i < 3; i++) {
		pos_out.v[i] *= T3DM_TO_N64_SCALE;
	}

	for (int i = 0; i < 3; i++) {
		fwrite_ef32(pos_out.v + i, file);
	}

	for (int i = 0; i < 3; i++) {
		fwrite_ef32(o->rotation.v + i, file);
	}

	for (int i = 0; i < 3; i++) {
		fwrite_ef32(o->scale.v + i, file);
	}
}

static void area_write_to_file(const area_t *a, FILE *file)
{
	collision_mesh_write_to_file(&a->colmesh, file);
	fwrite_ef16(&a->num_objects, file);
	for (uint16_t i = 0; i < a->num_objects; i++) {
		object_write_to_file(a->objects + i, file);
	}
}

static void scene_write_to_file(const scene_t *scn, FILE *file)
{
	fwrite_ef16(&scn->num_areas, file);
	for (uint16_t i = 0; i < scn->num_areas; i++) {
		area_write_to_file(scn->areas + i, file);
	}
	printf("Successfully wrote scene to file\n");
}

static void scene_process_area(area_t *a, const struct aiScene *aiscn,
			       const struct aiNode *n)
{
	a->num_objects = 0;
	a->objects = malloc(0);
	int num_col_meshes = 0;
	for (unsigned int i = 0; i < n->mNumChildren; i++) {
		const struct aiNode *ch = n->mChildren[i];

		if (!strncmp(ch->mName.data, "Col", 3)) {
			a->colmesh = collision_mesh_from_assimp(aiscn, ch);
			num_col_meshes++;
			if (num_col_meshes > 1) {
				exitf("ERROR: Area can only have "
				      "one collision mesh\n");
			}
		} else if (!strncmp(ch->mName.data, "Obj", 3)) {
			a->objects =
				realloc(a->objects,
					sizeof *a->objects * ++a->num_objects);
			object_t *onew = a->objects + a->num_objects - 1;
			strncpy(onew->name, ch->mName.data,
				OBJECT_NAME_MAX_LENGTH);
			onew->position.v[0] = ch->mTransformation.a4;
			onew->position.v[1] = ch->mTransformation.b4;
			onew->position.v[2] = ch->mTransformation.c4;
			onew->rotation = (T3DVec3){ { 0, 0, 0 } };
			onew->scale = (T3DVec3){ { 1, 1, 1 } };
		} else {
			exitf("ERROR: Invalid Item Type.\n");
		}
	}
}

static void assimp_scene_process_node(scene_t *scn, const struct aiScene *aiscn,
				      const struct aiNode *n)
{
	if (!strncmp("Area", n->mName.data, 4)) {
		scn->areas = realloc(scn->areas,
				     sizeof *scn->areas * ++scn->num_areas);
		scene_process_area(scn->areas + scn->num_areas - 1, aiscn, n);
	}

	for (unsigned int i = 0; i < n->mNumChildren; i++) {
		assimp_scene_process_node(scn, aiscn, n->mChildren[i]);
	}
}

static void scene_debug(const scene_t *s, const char *scn_path)
{
	printf("DEBUGGING SCENE '%s' (%d areas)\n", scn_path, s->num_areas);
	for (uint16_t i = 0; i < s->num_areas; i++) {
		area_t *a = s->areas + i;

		printf("\tArea %d (%d objects):\n", i, a->num_objects);
		printf("\t\tCollision Mesh (%d triangles)\n",
		       a->colmesh.num_triangles);
		printf("\t\t\tOffset: (%f, %f, %f)\n", a->colmesh.offset.v[0],
		       a->colmesh.offset.v[1], a->colmesh.offset.v[2]);
		for (uint16_t j = 0; j < a->colmesh.num_triangles; j++) {
			collision_triangle_t *tri = a->colmesh.triangles + j;

			printf("\t\t\tTri %d:\n", j);
			for (uint16_t k = 0; k < 3; k++) {
				printf("\t\t\t\tPos %d: (%f, %f, %f)\n", k,
				       tri->verts[k].pos[0],
				       tri->verts[k].pos[1],
				       tri->verts[k].pos[2]);
			}
			printf("\t\t\t\tNorm: (%f, %f, %f)\n", tri->norm[0],
			       tri->norm[1], tri->norm[2]);
		}

		for (uint16_t j = 0; j < a->num_objects; j++) {
			object_t *o = a->objects + j;

			printf("\t\tObject %d ('%s'):\n", j, o->name);
			printf("\t\t\tPosition: (%f, %f, %f):\n",
			       o->position.v[0], o->position.v[1],
			       o->position.v[2]);
			printf("\t\t\tRotation: (%f, %f, %f):\n",
			       o->rotation.v[0], o->rotation.v[1],
			       o->rotation.v[2]);
			printf("\t\t\tScale: (%f, %f, %f):\n", o->scale.v[0],
			       o->scale.v[1], o->scale.v[2]);
		}
	}
}

static scene_t handle_scene(const struct aiScene *aiscn, const char *scn_path)
{
	scene_t scn;
	scn.num_areas = 0;
	scn.areas = malloc(0);

	FILE *scn_file = fopen(scn_path, "wb");

	if (!scn_file) {
		exitf("Failed to write scene to '%s'\n", scn_path);
	}

	assimp_scene_process_node(&scn, aiscn, aiscn->mRootNode);
	scene_debug(&scn, scn_path);
	scene_write_to_file(&scn, scn_file);

	return scn;
}

int main(int argc, char **argv)
{
	char scn_path[512];
	const char *glb_path = argv[1];

	memset(scn_path, 0, 512);

	switch (argc) {
	default:
		exitf("Invalid number of arguments.\n");
		return 1;

	case 2:
		snprintf(scn_path, strlen(glb_path) - 3, "%s", glb_path);
		snprintf(scn_path + strlen(scn_path), 5, ".col");
		break;

	case 3:
		strncpy(scn_path, argv[2], 512);
		break;
	}

	const struct aiScene *scene =
		aiImportFile(glb_path, aiProcess_Triangulate |
					       aiProcess_JoinIdenticalVertices);

	if (!scene) {
		exitf("Failed to load scene from '%s'\n", glb_path);
	}

	/* transferring mesh over */
	// scene_t scn = scene_from_assimp(scene);
	handle_scene(scene, scn_path);
	// collision_mesh_t meshes_combined = collision_mesh_from_assimp(scene);

	/* writing collision data out to file */
	FILE *file = fopen(scn_path, "wb");

	if (!file) {
		exitf("Failed to open collision file from '%s'\n", scn_path);
	}

	return 0;
}
