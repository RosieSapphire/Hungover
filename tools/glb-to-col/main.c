#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include "../../include/config.h"
#include "../../include/engine/collision.h"

// #define RUN_TEST

static void exitf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	exit(EXIT_FAILURE);
}

static void combine_meshes(collision_mesh_t *out, const uint16_t array_len,
			   const collision_mesh_t *array)
{
	out->num_triangles = 0;
	out->triangles = malloc(0);
	for (uint16_t i = 0; i < array_len; i++) {
		const collision_mesh_t *m = array + i;

		out->num_triangles += m->num_triangles;
		out->triangles =
			realloc(out->triangles,
				sizeof *out->triangles * out->num_triangles);
		memcpy(out->triangles + (out->num_triangles - m->num_triangles),
		       m->triangles, sizeof *m->triangles * m->num_triangles);
	}
}

static void cleanup(uint16_t *num_meshes, collision_mesh_t **meshes_out,
		    FILE *file)
{
	/* cleanup */
	for (uint16_t i = 0; i < *num_meshes; i++) {
		collision_mesh_t *m = *meshes_out + i;

		free(m->triangles);
		m->triangles = NULL;
		m->num_triangles = 0;
	}
	free(*meshes_out);
	*meshes_out = NULL;
	fclose(file);
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

static void assimp_mesh_to_collision_mesh(collision_mesh_t *mesh_out,
					  const struct aiMesh *mesh_in)
{
	mesh_out->num_triangles = mesh_in->mNumFaces;
	mesh_out->triangles =
		calloc(mesh_out->num_triangles, sizeof *mesh_out->triangles);
	for (uint16_t i = 0; i < mesh_out->num_triangles; i++) {
		const struct aiFace *tri_in = mesh_in->mFaces + i;
		collision_triangle_t *tri_out = mesh_out->triangles + i;

		for (uint16_t j = 0; j < 3; j++) {
			collision_vertex_t *vert_out = tri_out->verts + j;
			const struct aiVector3D vert_in =
				mesh_in->mVertices[tri_in->mIndices[j]];

			vert_out->pos[0] = vert_in.x * T3DM_TO_N64_SCALE;
			vert_out->pos[1] = vert_in.y * T3DM_TO_N64_SCALE;
			vert_out->pos[2] = vert_in.z * T3DM_TO_N64_SCALE;
		}
		triangle_calc_normal(tri_out);
	}
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

static void write_to_file(const collision_mesh_t *mesh, FILE *file)
{
	fwrite_ef16(&mesh->num_triangles, file);
	for (uint16_t j = 0; j < mesh->num_triangles; j++) {
		collision_triangle_t *f = mesh->triangles + j;

		for (uint16_t k = 0; k < 3; k++) {
			collision_vertex_t *v = f->verts + k;

			for (uint16_t l = 0; l < 3; l++) {
				fwrite_ef32(v->pos + l, file);
			}
		}

		for (uint16_t k = 0; k < 3; k++) {
			fwrite_ef32(f->norm + k, file);
		}
	}
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

	/* transferring mesh over */
	uint16_t num_meshes = scene->mNumMeshes;
	collision_mesh_t *meshes_out = calloc(num_meshes, sizeof *meshes_out);
	for (int i = 0; i < num_meshes; i++) {
		const struct aiMesh *in = scene->mMeshes[i];
		collision_mesh_t *out = meshes_out + i;

		assimp_mesh_to_collision_mesh(out, in);
	}
	collision_mesh_t meshes_combined;
	combine_meshes(&meshes_combined, num_meshes, meshes_out);

	/* writing collision data out to file */
	FILE *file = fopen(col_path, "wb");

	if (!file) {
		exitf("Failed to open collision file from '%s'\n", col_path);
	}

	write_to_file(&meshes_combined, file);

	cleanup(&num_meshes, &meshes_out, file);

#ifdef RUN_TEST
	/* test by reading from file */

	file = fopen(col_path, "rb");

	if (!file) {
		exitf("TEST FAILED! Couldn't read file '%s'\n", col_path);
	}

	printf("\n!!!RUNNING GLB-TO-COL TEST CODE!!!\n");
	collision_mesh_t *m = &meshes_combined;
	fread_ef16(&m->num_triangles, file);
	printf("\n'%s' has %d triangles:\n", col_path, m->num_triangles);
	m->triangles = calloc(m->num_triangles, sizeof *m->triangles);
	for (uint16_t i = 0; i < m->num_triangles; i++) {
		collision_triangle_t *f = m->triangles + i;

		printf("\tFace (%d/%d):\n", i + 1, m->num_triangles);
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

#endif

	return 0;
}
