#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

typedef struct {
	float pos[3];
	float uv[2];
} vertex_t;

typedef struct {
	uint16_t num_verts, num_indis;
	vertex_t *verts;
	uint16_t *indis;
} mesh_t;

typedef struct {
	uint16_t num_meshes;
	mesh_t *meshes;
} scene_t;

static uint16_t uint16_endian_flip(uint16_t x)
{
	return (x >> 8) | (x << 8);
}

static float float_endian_flip(float x)
{
	uint32_t tmp = *((uint32_t *)&x);
	tmp = ((tmp << 8) & 0xFF00FF00) | ((tmp >> 8) & 0x00FF00FF); 
    	tmp = (tmp << 16) | (tmp >> 16);
	return *((float *)&tmp);
}

static void _print_usage(char *argv0)
{
	fprintf(stderr, "Usage: %s [input .blend] [output .scn]\n", argv0);
	exit(EXIT_FAILURE);
}

static vertex_t *_mesh_process_verts(const struct aiMesh *mesh)
{
	const uint16_t num_verts = mesh->mNumVertices;
	vertex_t *verts = malloc(sizeof(*verts) * num_verts);

	for(int i = 0; i < num_verts; i++) {
		const struct aiVector3D pos = mesh->mVertices[i];
		const struct aiVector3D uv = mesh->mTextureCoords[0][i];
		verts[i].pos[0] = pos.x;
		verts[i].pos[1] = pos.y;
		verts[i].pos[2] = pos.z;
		verts[i].uv[0] = uv.x;
		verts[i].uv[1] = uv.y;
	}

	return verts;
}

static uint16_t *_mesh_process_indis(const struct aiMesh *mesh)
{
	const int num_faces = mesh->mNumFaces;
	uint16_t *indis = malloc(sizeof(*indis) * num_faces * 3);

	for(int i = 0; i < num_faces; i++) {
		const struct aiFace face = mesh->mFaces[i];
		for(int j = 0; j < face.mNumIndices; j++) {
			indis[i * 3 + j] = face.mIndices[j];
		}
	}

	return indis;
}

static void _mesh_debug(const mesh_t *m)
{
	printf("\tVerts (%d):\n", m->num_verts);
	for(int i = 0; i < m->num_verts; i++) {
		const vertex_t vert = m->verts[i];
		printf("\tpos=(%f, %f, %f) uv=(%f, %f)\n",
				i, vert.pos[0], vert.pos[1], vert.pos[2],
				vert.uv[0], vert.uv[1]);
	}

	printf("\n\tIndis (%d):\n", m->num_indis);
	for(int i = 0; i < m->num_indis / 3; i++) {
		for(int j = 0; j < 3; j++)
			printf("\t%d", m->indis[i * 3 + j]);
		printf("\n");
	}
}

static void _scene_debug(const scene_t *s)
{
	for(int i = 0; i < s->num_meshes; i++) {
		printf("Mesh %d:\n", i);
		_mesh_debug(s->meshes + i);
	}
}

static mesh_t *_meshes_process(struct aiMesh **meshes, uint16_t num_meshes)
{
	mesh_t *meshes_conv = malloc(sizeof(*meshes_conv) * num_meshes);
	for(int i = 0; i < num_meshes; i++) {
		const struct aiMesh *mesh = meshes[i];
		vertex_t *verts = _mesh_process_verts(mesh);
		uint16_t *indis = _mesh_process_indis(mesh);

		meshes_conv[i].num_verts = mesh->mNumVertices;
		const int verts_size = sizeof(vertex_t) * mesh->mNumVertices;
		meshes_conv[i].verts = malloc(verts_size);
		memcpy(meshes_conv[i].verts, verts, verts_size);

		meshes_conv[i].num_indis = mesh->mNumFaces * 3;
		const int indis_size =
			sizeof(unsigned int) * mesh->mNumFaces * 3;
		meshes_conv[i].indis = malloc(indis_size);
		memcpy(meshes_conv[i].indis, indis, indis_size);
	}

	return meshes_conv;
}

static void _scene_write(const scene_t *scene, const char *outpath)
{
	FILE *file = fopen(outpath, "wb");
	if(file) {
		fclose(file);
		remove(outpath);
		file = fopen(outpath, "wb");
	} else
		fprintf(stderr, "Failed to write to file '%s'\n", outpath);

	uint16_t num_meshes_flip = uint16_endian_flip(scene->num_meshes);
	fwrite(&num_meshes_flip, sizeof(num_meshes_flip), 1, file);

	for(int i = 0; i < scene->num_meshes; i++) {
		mesh_t *mesh = scene->meshes + i;
		const uint16_t num_verts_flip =
			uint16_endian_flip(mesh->num_verts);
		fwrite(&num_verts_flip, sizeof(uint16_t), 1, file);
		for(int j = 0; j < mesh->num_verts; j++) {
			vertex_t vert = mesh->verts[j];
			for(int k = 0; k < 3; k++)
				vert.pos[k] = float_endian_flip(vert.pos[k]);

			for(int k = 0; k < 2; k++)
				vert.uv[k] = float_endian_flip(vert.uv[k]);

			fwrite(&vert, sizeof(vertex_t), 1, file);
		}

		const uint16_t num_indis_flip =
			uint16_endian_flip(mesh->num_indis);
		fwrite(&num_indis_flip, sizeof(uint16_t), 1, file);
		for(int j = 0; j < mesh->num_indis; j++) {
			mesh->indis[j] = uint16_endian_flip(mesh->indis[j]);
			fwrite(mesh->indis + j, sizeof(uint16_t), 1, file);
		}
	}

	fclose(file);
}

static void _import_test(const char *path)
{
	FILE *file = fopen(path, "rb");
	scene_t scene;
	fread(&scene.num_meshes, sizeof(uint16_t), 1, file);
	scene.num_meshes = uint16_endian_flip(scene.num_meshes);
	scene.meshes = malloc(sizeof(mesh_t) * scene.num_meshes);
	printf("File has %d meshes.\n", scene.num_meshes);
	for(int i = 0; i < scene.num_meshes; i++) {
		mesh_t *mesh = scene.meshes + i;
		fread(&mesh->num_verts, sizeof(uint16_t), 1, file);
		mesh->num_verts = uint16_endian_flip(mesh->num_verts);
		mesh->verts = malloc(sizeof(vertex_t) * mesh->num_verts);
		for(int j = 0; j < mesh->num_verts; j++) {
			vertex_t *vert = mesh->verts + j;
			fread(vert, sizeof(vertex_t), 1, file);
			for(int k = 0; k < 3; k++)
				vert->pos[k] = float_endian_flip(vert->pos[k]);

			for(int k = 0; k < 2; k++)
				vert->uv[k] = float_endian_flip(vert->uv[k]);
		}

		fread(&mesh->num_indis, sizeof(uint16_t), 1, file);
		mesh->num_indis = uint16_endian_flip(mesh->num_indis);
		mesh->indis = malloc(sizeof(uint16_t) * mesh->num_indis);
		for(int j = 0; j < mesh->num_indis; j++) {
			fread(mesh->indis + j, sizeof(uint16_t), 1, file);
			mesh->indis[j] = uint16_endian_flip(mesh->indis[j]);
		}
	}
	fclose(file);
	_scene_debug(&scene);
}

int main(int argc, char **argv)
{
	if(argc != 3)
		_print_usage(argv[0]);

	const char *path_in = argv[1];
	const int ppflags = aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate | aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials;
	const struct aiScene *scene = aiImportFile(path_in, ppflags);
	if(!scene) {
		fprintf(stderr, "Assimp Error (%s): %s\n", path_in,
				aiGetErrorString());
		exit(EXIT_FAILURE);
	}

	scene_t sceneout;
	sceneout.num_meshes = scene->mNumMeshes;
	sceneout.meshes = _meshes_process(scene->mMeshes, sceneout.num_meshes);

	const char *path_out = argv[2];
	_scene_write(&sceneout, path_out);
	// _import_test(path_out);

	return 0;
}
