#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include "../../include/config.h"
#include "../../include/engine/collision.h"

#define RUN_TEST

static void exitf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	exit(EXIT_FAILURE);
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

static void write_to_file(const collision_mesh_t *m, FILE *file)
{
	fwrite_ef16(&m->num_triangles, file);
	for (uint16_t i = 0; i < m->num_triangles; i++) {
		collision_triangle_t *f = m->triangles + i;

		for (uint16_t j = 0; j < 3; j++) {
			collision_vertex_t *v = f->verts + j;

			for (uint16_t k = 0; k < 3; k++) {
				fwrite_ef32(v->pos + k, file);
			}
		}

		for (uint16_t j = 0; j < 3; j++) {
			fwrite_ef32(f->norm + j, file);
		}
	}
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

int main(int argc, char **argv)
{
	char col_path[512];
	const char *glb_path = argv[1];

	memset(col_path, 0, 512);

	switch (argc) {
	default:
		exitf("Invalid number of arguments.\n");
		return 1;

	case 2:
		snprintf(col_path, strlen(glb_path) - 3, "%s", glb_path);
		snprintf(col_path + strlen(col_path), 5, ".col");
		break;

	case 3:
		strncpy(col_path, argv[2], 512);
		break;
	}

	const struct aiScene *scene =
		aiImportFile(glb_path, aiProcess_Triangulate |
					       aiProcess_JoinIdenticalVertices);

	if (!scene) {
		exitf("Failed to load scene from '%s'\n", glb_path);
	}

	/*
	printf("\nGenerating collision from GLB File '%s':\n", glb_path);
	printf("File has %d mesh(es) [irrelevant for now].\n",
	       scene->mNumMeshes);
	       */

	/* TODO: Maybe make this consolidate multiple meshes. idk */

	/* transferring mesh over */
	const struct aiMesh *mesh_in = scene->mMeshes[0];
	collision_mesh_t mesh_out;
	/*
	printf("Mesh '%s' has %d triangles:\n", mesh_in->mName.data,
	       mesh_in->mNumFaces);
	       */
	mesh_out.num_triangles = mesh_in->mNumFaces;
	mesh_out.triangles =
		calloc(mesh_out.num_triangles, sizeof *mesh_out.triangles);
	for (uint16_t i = 0; i < mesh_out.num_triangles; i++) {
		const struct aiFace *tri_in = mesh_in->mFaces + i;
		collision_triangle_t *tri_out = mesh_out.triangles + i;

		// printf("\tFace (%d/%d):\n", i + 1, mesh_out.num_triangles);
		for (uint16_t j = 0; j < 3; j++) {
			collision_vertex_t *vert_out = tri_out->verts + j;
			const struct aiVector3D vert_in =
				mesh_in->mVertices[tri_in->mIndices[j]];

			vert_out->pos[0] = vert_in.x * T3DM_TO_N64_SCALE;
			vert_out->pos[1] = vert_in.y * T3DM_TO_N64_SCALE;
			vert_out->pos[2] = vert_in.z * T3DM_TO_N64_SCALE;
			/*
			printf("\t\tP%d: (%f, %f, %f)\n", j, vert_out->pos[0],
			       vert_out->pos[1], vert_out->pos[2]);
			       */
		}
		triangle_calc_normal(tri_out);
		/*
		printf("\t\tNORM: (%f, %f, %f)\n", tri_out->norm[0],
		       tri_out->norm[1], tri_out->norm[2]);
		       */
	}

	/* writing collision data out to file */
	// printf("\nWriting collision data to '%s'...\n", col_path);

	FILE *file = fopen(col_path, "wb");

	if (!file) {
		exitf("Failed to open collision file from '%s'\n", col_path);
	}

	write_to_file(&mesh_out, file);

	/* cleanup */
	free(mesh_out.triangles);
	mesh_out.triangles = NULL;
	mesh_out.num_triangles = 0;
	fclose(file);

#ifdef RUN_TEST
	/* test by reading from file */

	file = fopen(col_path, "rb");

	if (!file) {
		exitf("TEST FAILED! Couldn't read file '%s'\n", col_path);
	}

	printf("\n!!!RUNNING GLB-TO-COL TEST CODE!!!\n");
	fread_ef16(&mesh_out.num_triangles, file);
	printf("\n'%s' has %d triangles:\n", col_path, mesh_out.num_triangles);
	mesh_out.triangles =
		calloc(mesh_out.num_triangles, sizeof *mesh_out.triangles);
	for (uint16_t i = 0; i < mesh_out.num_triangles; i++) {
		collision_triangle_t *f = mesh_out.triangles + i;

		printf("\tFace (%d/%d):\n", i + 1, mesh_out.num_triangles);
		for (uint16_t j = 0; j < 3; j++) {
			collision_vertex_t *v = f->verts + j;

			for (uint16_t k = 0; k < 3; k++) {
				fread_ef32(v->pos + k, file);
			}
			printf("\t\tP%d: (%f, %f, %f)\n", j, v->pos[0],
			       v->pos[1], v->pos[2]);
		}

		for (uint16_t j = 0; j < 3; j++) {
			fread_ef32(f->norm + j, file);
		}
		printf("\t\tNORM: (%f, %f, %f)\n", f->norm[0], f->norm[1],
		       f->norm[2]);
	}

	/* cleanup */
	free(mesh_out.triangles);
	mesh_out.triangles = NULL;
	mesh_out.num_triangles = 0;
	fclose(file);

#endif

	return 0;
}
