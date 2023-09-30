#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#define NAME_MAX_LEN 32

typedef struct {
	float pos[3];
	float uv[2];
} vertex_t;

typedef struct {
	char name[NAME_MAX_LEN];
	uint16_t num_verts, num_indis;
	vertex_t *verts;
	uint16_t *indis;
} mesh_t;

typedef struct node_t node_t;
struct node_t {
	char name[NAME_MAX_LEN];
	uint16_t mesh_index;
	uint16_t num_children;
	node_t *children;
};

typedef struct {
	uint16_t frame;
	float vec[3];
} vec3_key_t;

typedef struct {
	uint16_t frame;
	float vec[4];
} vec4_key_t;

typedef struct {
	char name[NAME_MAX_LEN];
	uint16_t mesh_index, length, num_pos, num_rot, num_sca;
	vec3_key_t *pos;
	vec4_key_t *rot;
	vec3_key_t *sca;
} animation_t;

typedef struct {
	uint16_t num_meshes;
	mesh_t *meshes;
	uint16_t num_anims;
	animation_t *anims;
	node_t root_node;
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
		for(unsigned int j = 0; j < face.mNumIndices; j++) {
			indis[i * 3 + j] = face.mIndices[j];
		}
	}

	return indis;
}

static mesh_t *_meshes_process(const struct aiScene *scene)
{
	mesh_t *meshes_conv = malloc(sizeof(*meshes_conv) * scene->mNumMeshes);
	for(uint16_t i = 0; i < scene->mNumMeshes; i++) {
		const struct aiMesh *mesh = scene->mMeshes[i];
		strncpy(meshes_conv[i].name, mesh->mName.data,
				mesh->mName.length + 1);
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

static void _meshes_write(const scene_t *scene, FILE *file)
{
	uint16_t num_meshes_flip = uint16_endian_flip(scene->num_meshes);
	fwrite(&num_meshes_flip, sizeof(num_meshes_flip), 1, file);

	for(uint16_t i = 0; i < scene->num_meshes; i++) {
		mesh_t *mesh = scene->meshes + i;

		fwrite(mesh->name, sizeof(char), NAME_MAX_LEN, file);

		/* flipping and storing the verts */
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

		/* flipping and storing the indis */
		const uint16_t num_indis_flip =
			uint16_endian_flip(mesh->num_indis);
		fwrite(&num_indis_flip, sizeof(uint16_t), 1, file);
		for(int j = 0; j < mesh->num_indis; j++) {
			mesh->indis[j] = uint16_endian_flip(mesh->indis[j]);
			fwrite(mesh->indis + j, sizeof(uint16_t), 1, file);
		}
	}

}

static void _anims_write(const scene_t *scene, FILE *file)
{
	uint16_t num_anims_flip = uint16_endian_flip(scene->num_anims);
	fwrite(&num_anims_flip, sizeof(num_anims_flip), 1, file);

	for(uint16_t i = 0; i < scene->num_anims; i++) {
		animation_t *anim = scene->anims + i;
		fwrite(anim->name, sizeof(char), NAME_MAX_LEN, file);
		uint16_t mesh_index_flip = uint16_endian_flip(anim->mesh_index);
		fwrite(&mesh_index_flip, sizeof(uint16_t), 1, file);
		uint16_t length_flip = uint16_endian_flip(anim->length);
		fwrite(&length_flip, sizeof(uint16_t), 1, file);
		uint16_t num_pos_flip = uint16_endian_flip(anim->num_pos);
		fwrite(&num_pos_flip, sizeof(uint16_t), 1, file);
		uint16_t num_rot_flip = uint16_endian_flip(anim->num_rot);
		fwrite(&num_rot_flip, sizeof(uint16_t), 1, file);
		uint16_t num_sca_flip = uint16_endian_flip(anim->num_sca);
		fwrite(&num_sca_flip, sizeof(uint16_t), 1, file);

		for(int j = 0; j < anim->num_pos; j++) {
			uint16_t frame_flip =
				uint16_endian_flip(anim->pos[j].frame);
			fwrite(&frame_flip, sizeof(uint16_t), 1, file);
			float vec_flip[3] = {
				float_endian_flip(anim->pos[j].vec[0]),
				float_endian_flip(anim->pos[j].vec[1]),
				float_endian_flip(anim->pos[j].vec[2]),
			};
			fwrite(vec_flip, sizeof(float), 3, file);
		}

		for(int j = 0; j < anim->num_rot; j++) {
			uint16_t frame_flip =
				uint16_endian_flip(anim->pos[j].frame);
			fwrite(&frame_flip, sizeof(uint16_t), 1, file);
			float vec_flip[4] = {
				float_endian_flip(anim->rot[j].vec[0]),
				float_endian_flip(anim->rot[j].vec[1]),
				float_endian_flip(anim->rot[j].vec[2]),
				float_endian_flip(anim->rot[j].vec[3]),
			};
			fwrite(vec_flip, sizeof(float), 4, file);
		}

		for(int j = 0; j < anim->num_sca; j++) {
			uint16_t frame_flip =
				uint16_endian_flip(anim->pos[j].frame);
			fwrite(&frame_flip, sizeof(uint16_t), 1, file);
			float vec_flip[3] = {
				float_endian_flip(anim->sca[j].vec[0]),
				float_endian_flip(anim->sca[j].vec[1]),
				float_endian_flip(anim->sca[j].vec[2]),
			};
			fwrite(vec_flip, sizeof(float), 3, file);
		}
	}
}

static void _node_write(node_t *node, FILE *file)
{
	fwrite(node->name, sizeof(char), NAME_MAX_LEN, file);
	node->mesh_index = uint16_endian_flip(node->mesh_index);
	fwrite(&node->mesh_index, sizeof(uint16_t), 1, file);
	node->num_children = uint16_endian_flip(node->num_children);
	fwrite(&node->num_children, sizeof(uint16_t), 1, file);
	for(int i = 0; i < uint16_endian_flip(node->num_children); i++)
		_node_write(&node->children[i], file);
}

static void _scene_write(scene_t *scene, const char *outpath)
{
	FILE *file = fopen(outpath, "wb");
	if(file) {
		fclose(file);
		remove(outpath);
		file = fopen(outpath, "wb");
	} else
		fprintf(stderr, "Failed to write to file '%s'\n", outpath);

	_meshes_write(scene, file);
	_anims_write(scene, file);
	_node_write(&scene->root_node, file);

	fclose(file);
}

static void _node_import(scene_t *s, node_t *n, FILE *file)
{
	fread(n->name, sizeof(char), NAME_MAX_LEN, file);
	fread(&n->mesh_index, sizeof(uint16_t), 1, file);
	fread(&n->num_children, sizeof(uint16_t), 1, file);
	for(int i = 0; i < uint16_endian_flip(n->num_children); i++)
		_node_import(s, n->children + i, file);
}

static void _import_test(const char *path)
{
	FILE *file = fopen(path, "rb");
	scene_t scene;
	fread(&scene.num_meshes, sizeof(uint16_t), 1, file);
	scene.num_meshes = uint16_endian_flip(scene.num_meshes);
	scene.meshes = malloc(sizeof(mesh_t) * scene.num_meshes);
	for(int i = 0; i < scene.num_meshes; i++) {
		mesh_t *mesh = scene.meshes + i;
		fread(&mesh->name, sizeof(char), NAME_MAX_LEN, file);
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

	fread(&scene.num_anims, sizeof(uint16_t), 1, file);
	scene.num_anims = uint16_endian_flip(scene.num_anims);
	scene.anims = malloc(sizeof(animation_t) * scene.num_anims);
	for(int i = 0; i < scene.num_anims; i++) {
		animation_t *anim = scene.anims + i;
		fread(&anim->name, sizeof(char), NAME_MAX_LEN, file);
		fread(&anim->length, sizeof(uint16_t), 1, file);
		anim->length = uint16_endian_flip(anim->length);
		fread(&anim->num_pos, sizeof(uint16_t), 1, file);
		anim->num_pos = uint16_endian_flip(anim->num_pos);
		fread(&anim->num_rot, sizeof(uint16_t), 1, file);
		anim->num_rot = uint16_endian_flip(anim->num_rot);
		fread(&anim->num_sca, sizeof(uint16_t), 1, file);
		anim->num_sca = uint16_endian_flip(anim->num_sca);

		anim->pos = malloc(sizeof(vec3_key_t) * anim->num_pos);
		for(int j = 0; j < anim->num_pos; j++) {
			fread(&anim->pos[j].frame, sizeof(uint16_t), 1, file);
			anim->pos[j].frame =
				uint16_endian_flip(anim->pos[j].frame);
			fread(anim->pos[j].vec, sizeof(float), 3, file);
			for(int k = 0; k < 3; k++)
				anim->pos[j].vec[k] = 
					float_endian_flip(anim->pos[j].vec[k]);
		}

		anim->rot = malloc(sizeof(vec4_key_t) * anim->num_rot);
		for(int j = 0; j < anim->num_rot; j++) {
			fread(&anim->rot[j].frame, sizeof(uint16_t), 1, file);
			anim->rot[j].frame =
				uint16_endian_flip(anim->rot[j].frame);
			fread(anim->rot[j].vec, sizeof(float), 4, file);
			for(int k = 0; k < 4; k++)
				anim->rot[j].vec[k] = 
					float_endian_flip(anim->rot[j].vec[k]);
		}

		anim->sca = malloc(sizeof(vec3_key_t) * anim->num_sca);
		for(int j = 0; j < anim->num_sca; j++) {
			fread(&anim->sca[j].frame, sizeof(uint16_t), 1, file);
			anim->sca[j].frame =
				uint16_endian_flip(anim->sca[j].frame);
			fread(anim->sca[j].vec, sizeof(float), 3, file);
			for(int k = 0; k < 3; k++)
				anim->sca[j].vec[k] = 
					float_endian_flip(anim->sca[j].vec[k]);
		}
	}

	_node_import(&scene, &scene.root_node, file);

	fclose(file);
}

static node_t _node_process(const struct aiNode *n)
{
	node_t node;
	strncpy(node.name, n->mName.data, n->mName.length + 1);
	node.mesh_index = n->mNumMeshes ? n->mMeshes[0] : 0xFFFF;
	node.num_children = n->mNumChildren;
	node.children = malloc(sizeof(node) * node.num_children);
	for(int i = 0; i < node.num_children; i++)
		node.children[i] = _node_process(n->mChildren[i]);

	return node;
}

static uint16_t _mesh_name_to_index(const char *name,
		mesh_t *meshes, int num_meshes)
{
	for(int i = 0; i < num_meshes; i++) {
		printf("Trying '%s'...\n", meshes[i].name);
		if(strcmp(meshes[i].name, name) == 0) {
			printf("\tFound '%s'!\n", name);
			return i;
		}
	}

	fprintf(stderr, "Mesh wasn't found with name '%s'\n", name);
	return -1;
}

static void _anim_process(animation_t *anim, const struct aiAnimation *a,
		mesh_t *meshes, uint16_t num_meshes, uint16_t index)
{
	const struct aiNodeAnim *n = a->mChannels[index];
	strncpy(anim->name, a->mName.data, a->mName.length + 1);
	anim->mesh_index = _mesh_name_to_index(n->mNodeName.data,
			meshes, num_meshes);
	anim->length = roundf((a->mDuration / a->mTicksPerSecond) * 24);

	anim->num_pos = n->mNumPositionKeys;
	anim->pos = malloc(sizeof(vec3_key_t) * anim->num_pos);
	for(uint16_t i = 0; i < anim->num_pos; i++) {
		const struct aiVectorKey pi = n->mPositionKeys[i];
		vec3_key_t *po = anim->pos + i;
		po->frame = roundf((pi.mTime / a->mTicksPerSecond) * 24);
		memcpy(po->vec, &pi.mValue.x, sizeof(float) * 3);
	}

	anim->num_rot = n->mNumRotationKeys;
	anim->rot = malloc(sizeof(vec4_key_t) * anim->num_rot);
	for(uint16_t i = 0; i < anim->num_rot; i++) {
		const struct aiQuatKey ri = n->mRotationKeys[i];
		vec4_key_t *ro = anim->rot + i;
		ro->frame = roundf((ri.mTime / a->mTicksPerSecond) * 24);
		ro->vec[0] = ri.mValue.x;
		ro->vec[1] = ri.mValue.y;
		ro->vec[2] = ri.mValue.z;
		ro->vec[3] = ri.mValue.w;
	}

	anim->num_sca = n->mNumScalingKeys;
	anim->sca = malloc(sizeof(vec3_key_t) * anim->num_rot);
	for(uint16_t i = 0; i < anim->num_sca; i++) {
		const struct aiVectorKey si = n->mScalingKeys[i];
		vec3_key_t *so = anim->sca + i;
		so->frame = roundf((si.mTime / a->mTicksPerSecond) * 24);
		memcpy(so->vec, &si.mValue.x, sizeof(float) * 3);
	}
}

static animation_t *_anims_process(const struct aiScene *s,
		mesh_t *meshes, uint16_t num_meshes)
{
	animation_t *anims = malloc(sizeof(animation_t) * s->mNumAnimations);
	for(uint16_t i = 0; i < s->mNumAnimations; i++)
		for(uint16_t j = 0; j < s->mAnimations[i]->mNumChannels; j++)
			_anim_process(anims + i, s->mAnimations[i],
					meshes, num_meshes, j);

	return anims;
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
	sceneout.meshes = _meshes_process(scene);
	sceneout.num_anims = scene->mNumAnimations;
	sceneout.anims = _anims_process(scene,
			sceneout.meshes, sceneout.num_meshes);
	sceneout.root_node = _node_process(scene->mRootNode);

	const char *path_out = argv[2];
	_scene_write(&sceneout, path_out);
	_import_test(path_out);

	return 0;
}
