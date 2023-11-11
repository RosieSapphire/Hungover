#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#define NAME_MAX_LEN 32

#define MAX_SCENE_TEXS 8
#define TEX_PATH_MAX_LEN 64

typedef struct {
	float pos[3];
	float uv[2];
} vertex_t;

typedef struct {
	char name[NAME_MAX_LEN];
	uint16_t num_verts, num_indis;
	vertex_t *verts;
	uint16_t *indis;
	uint16_t tex_index;
} mesh_t;

typedef struct node_t node_t;
struct node_t {
	char name[NAME_MAX_LEN];
	uint16_t mesh_index;
	float mat[4 * 4];
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
	uint16_t num_tex_paths;
	char tex_paths[MAX_SCENE_TEXS][TEX_PATH_MAX_LEN];
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

static void _mesh_process(mesh_t *mo, const struct aiMesh *mi)
{
	strncpy(mo->name, mi->mName.data, mi->mName.length + 1);
	mo->num_verts = mi->mNumVertices;
	mo->verts = malloc(mo->num_verts * sizeof(vertex_t));
	for(uint16_t i = 0; i < mo->num_verts; i++) {
		vertex_t *v = mo->verts + i;
		memcpy(v->pos, &mi->mVertices[i].x, sizeof(float) * 3);
		memcpy(v->uv, &mi->mTextureCoords[0][i], sizeof(float) * 3);
	}

	mo->num_indis = mi->mNumFaces * 3;
	mo->indis = malloc(mo->num_indis * sizeof(uint16_t));
	for(uint16_t i = 0; i < mi->mNumFaces; i++)
		for(uint16_t j = 0; j < mi->mFaces[i].mNumIndices; j++)
			mo->indis[i * 3 + j] = mi->mFaces[i].mIndices[j];

	mo->tex_index = mi->mMaterialIndex;
}

static uint16_t _mesh_index_from_name(const char *name, const mesh_t *meshes,
		const uint16_t num_meshes)
{
	for(uint16_t i = 0; i < num_meshes; i++)
		if(strcmp(name, meshes[i].name) == 0)
			return i;

	return 0xFFFF;
}

static void _anim_process(animation_t *ao, const scene_t *so,
		const struct aiAnimation *ai)
{
	/* acting */
	strncpy(ao->name, ai->mName.data, ai->mName.length + 1);
	ao->mesh_index = _mesh_index_from_name(
	        	ai->mChannels[0]->mNodeName.data,
	        	so->meshes, so->num_meshes);
	// ao->length = (roundf(ai->mDuration / ai->mTicksPerSecond) * 24) + 1;
	uint16_t nums[3] = {
		ao->num_pos = ai->mChannels[0]->mNumPositionKeys,
		ao->num_rot = ai->mChannels[0]->mNumRotationKeys,
		ao->num_sca = ai->mChannels[0]->mNumScalingKeys,
	};
	uint16_t max = 0;
	for(int i = 0; i < 3; i++) {
		if(max < nums[i])
			max = nums[i];
		ao->length = max;
	}

	ao->pos = malloc(sizeof(vec3_key_t) * ao->num_pos);
	for(uint16_t i = 0; i < ao->num_pos; i++) {
		ao->pos[i].frame = (uint16_t)roundf(
				(ai->mChannels[0]->mPositionKeys[i].mTime
				 / ai->mTicksPerSecond) * 24);
		ao->pos[i].vec[0] = ai->mChannels[0]->mPositionKeys[i].mValue.x;
		ao->pos[i].vec[1] = ai->mChannels[0]->mPositionKeys[i].mValue.y;
		ao->pos[i].vec[2] = ai->mChannels[0]->mPositionKeys[i].mValue.z;
	}

	ao->rot = malloc(sizeof(vec4_key_t) * ao->num_rot);
	for(uint16_t i = 0; i < ao->num_rot; i++) {
		ao->rot[i].frame = (uint16_t)roundf(
				(ai->mChannels[0]->mRotationKeys[i].mTime
				 / ai->mTicksPerSecond) * 24);
		ao->rot[i].vec[0] = ai->mChannels[0]->mRotationKeys[i].mValue.x;
		ao->rot[i].vec[1] = ai->mChannels[0]->mRotationKeys[i].mValue.y;
		ao->rot[i].vec[2] = ai->mChannels[0]->mRotationKeys[i].mValue.z;
		ao->rot[i].vec[3] = ai->mChannels[0]->mRotationKeys[i].mValue.w;
	}

	ao->sca = malloc(sizeof(vec3_key_t) * ao->num_sca);
	for(uint16_t i = 0; i < ao->num_sca; i++) {
		ao->sca[i].frame = (uint16_t)roundf(
				(ai->mChannels[0]->mScalingKeys[i].mTime
				 / ai->mTicksPerSecond) * 24);
		ao->sca[i].vec[0] = ai->mChannels[0]->mScalingKeys[i].mValue.x;
		ao->sca[i].vec[1] = ai->mChannels[0]->mScalingKeys[i].mValue.y;
		ao->sca[i].vec[2] = ai->mChannels[0]->mScalingKeys[i].mValue.z;
	}
}

static void _matrix_transpose(float *in, float *out)
{
	for (int i = 0; i < 4; i++)
        	for (int j = 0; j < 4; j++)
			out[j * 4 + i] = in[i * 4 + j];
}

static void _node_process(node_t *no, const struct aiNode *ni)
{
	strncpy(no->name, ni->mName.data, ni->mName.length + 1);
	no->mesh_index = ni->mNumMeshes ? ni->mMeshes[0] : 0xFFFF;
	_matrix_transpose((float *)&ni->mTransformation, no->mat);
	no->num_children = ni->mNumChildren;
	no->children = malloc(sizeof(node_t) * no->num_children);
	for(int i = 0; i < no->num_children; i++)
		_node_process(no->children + i, ni->mChildren[i]);
}

static void _scene_process(scene_t *so, const struct aiScene *si)
{
	so->num_meshes = si->mNumMeshes;
	so->meshes = malloc(so->num_meshes * sizeof(mesh_t));
	for(uint16_t i = 0; i < so->num_meshes; i++)
		_mesh_process(so->meshes + i, si->mMeshes[i]);

	so->num_anims = si->mNumAnimations;
	so->anims = malloc(so->num_anims * sizeof(animation_t));
	for(uint16_t i = 0; i < so->num_anims; i++)
		_anim_process(so->anims + i, so, si->mAnimations[i]);

	_node_process(&so->root_node, si->mRootNode);

	so->num_tex_paths = si->mNumMaterials;
	for(int i = 0; i < so->num_tex_paths; i++) {
		const struct aiMaterial *mat = si->mMaterials[i];
		struct aiString tex_path;
		aiGetMaterialString(mat, AI_MATKEY_NAME, &tex_path);
		memcpy(so->tex_paths[i], tex_path.data, tex_path.length + 1);
	}
}

static void _mesh_write(const mesh_t *m, FILE *file)
{
	fwrite(m->name, sizeof(char), NAME_MAX_LEN, file);
	uint16_t num_verts_flip = uint16_endian_flip(m->num_verts);
	fwrite(&num_verts_flip, sizeof(uint16_t), 1, file);
	for(uint16_t i = 0; i < m->num_verts; i++) {
		float pflip[3] = {
			float_endian_flip(m->verts[i].pos[0]),
			float_endian_flip(m->verts[i].pos[1]),
			float_endian_flip(m->verts[i].pos[2])
		};
		fwrite(pflip, sizeof(float), 3, file);

		float uflip[2] = {
			float_endian_flip(m->verts[i].uv[0]),
			float_endian_flip(m->verts[i].uv[1]),
		};
		fwrite(uflip, sizeof(float), 2, file);
	}

	uint16_t num_indis_flip = uint16_endian_flip(m->num_indis);
	fwrite(&num_indis_flip, sizeof(uint16_t), 1, file);
	for(uint16_t i = 0; i < m->num_indis; i++) {
		uint16_t iflip = uint16_endian_flip(m->indis[i]);
		fwrite(&iflip, sizeof(uint16_t), 1, file);
	}

	uint16_t tex_index_flip = uint16_endian_flip(m->tex_index);
	fwrite(&tex_index_flip, sizeof(uint16_t), 1, file);
}

static void _anim_write(const animation_t *a, FILE *file)
{
	fwrite(a->name, sizeof(char), NAME_MAX_LEN, file);
	uint16_t mesh_index_flip = uint16_endian_flip(a->mesh_index);
	fwrite(&mesh_index_flip, sizeof(uint16_t), 1, file);
	uint16_t length_flip = uint16_endian_flip(a->length);
	fwrite(&length_flip, sizeof(uint16_t), 1, file);
	uint16_t num_pos_flip = uint16_endian_flip(a->num_pos);
	fwrite(&num_pos_flip, sizeof(uint16_t), 1, file);
	uint16_t num_rot_flip = uint16_endian_flip(a->num_rot);
	fwrite(&num_rot_flip, sizeof(uint16_t), 1, file);
	uint16_t num_sca_flip = uint16_endian_flip(a->num_sca);
	fwrite(&num_sca_flip, sizeof(uint16_t), 1, file);

	for(uint16_t i = 0; i < a->num_pos; i++) {
		uint16_t frame_flip = uint16_endian_flip(a->pos[i].frame);
		fwrite(&frame_flip, sizeof(uint16_t), 1, file);
		float v[3] = {
			float_endian_flip(a->pos[i].vec[0]),
			float_endian_flip(a->pos[i].vec[1]),
			float_endian_flip(a->pos[i].vec[2])
		};
		fwrite(v, sizeof(float), 3, file);
	}

	for(uint16_t i = 0; i < a->num_rot; i++) {
		uint16_t frame_flip = uint16_endian_flip(a->rot[i].frame);
		fwrite(&frame_flip, sizeof(uint16_t), 1, file);
		float v[4] = {
			float_endian_flip(a->rot[i].vec[0]),
			float_endian_flip(a->rot[i].vec[1]),
			float_endian_flip(a->rot[i].vec[2]),
			float_endian_flip(a->rot[i].vec[3])
		};
		fwrite(v, sizeof(float), 4, file);
	}

	for(uint16_t i = 0; i < a->num_sca; i++) {
		uint16_t frame_flip = uint16_endian_flip(a->sca[i].frame);
		fwrite(&frame_flip, sizeof(uint16_t), 1, file);
		float v[3] = {
			float_endian_flip(a->sca[i].vec[0]),
			float_endian_flip(a->sca[i].vec[1]),
			float_endian_flip(a->sca[i].vec[2])
		};
		fwrite(v, sizeof(float), 3, file);
	}
}

static void _node_write(const node_t *n, FILE *file)
{
	fwrite(n->name, sizeof(char), NAME_MAX_LEN, file);
	uint16_t mesh_index_flip = uint16_endian_flip(n->mesh_index);
	fwrite(&mesh_index_flip, sizeof(uint16_t), 1, file);
	float mat_flip[4 * 4];
	for(int i = 0; i < 16; i++)
		mat_flip[i] = float_endian_flip(n->mat[i]);

	fwrite(mat_flip, sizeof(float), 4 * 4, file);
	uint16_t num_children_flip = uint16_endian_flip(n->num_children);
	fwrite(&num_children_flip, sizeof(uint16_t), 1, file);
	for(int i = 0; i < n->num_children; i++)
		_node_write(n->children + i, file);
}

static void _scene_write(const scene_t *s, const char *outpath)
{
	FILE *file = fopen(outpath, "wb");
	if(file) {
		fclose(file);
		remove(outpath);
		file = fopen(outpath, "wb");
	} else {
		fprintf(stderr, "Failed to write to file '%s' (%d) %s\n",
				outpath, errno, strerror(errno));
		exit(1);
	}

	int num_meshes_flip = uint16_endian_flip(s->num_meshes);
	fwrite(&num_meshes_flip, sizeof(uint16_t), 1, file);
	for(uint16_t i = 0; i < s->num_meshes; i++)
		_mesh_write(s->meshes + i, file);

	int num_anims_flip = uint16_endian_flip(s->num_anims);
	fwrite(&num_anims_flip, sizeof(uint16_t), 1, file);
	for(uint16_t i = 0; i < s->num_anims; i++)
		_anim_write(s->anims + i, file);

	_node_write(&s->root_node, file);

	int num_tex_paths_flip = uint16_endian_flip(s->num_tex_paths);
	fwrite(&num_tex_paths_flip, sizeof(uint16_t), 1, file);
	for(uint16_t i = 0; i < s->num_tex_paths; i++)
		fwrite(s->tex_paths[i], sizeof(char), TEX_PATH_MAX_LEN, file);

	fclose(file);
}

static void _mesh_read(mesh_t *m, FILE *file)
{
	fread(m->name, sizeof(char), NAME_MAX_LEN, file);
	fread(&m->num_verts, sizeof(uint16_t), 1, file);
	m->num_verts = uint16_endian_flip(m->num_verts);
	m->verts = malloc(sizeof(vertex_t) * m->num_verts);
	for(int i = 0; i < m->num_verts; i++) {
		fread(m->verts[i].pos, sizeof(float), 3, file);
		for(int j = 0; j < 3; j++)
			m->verts[i].pos[j] =
				float_endian_flip(m->verts[i].pos[j]);

		fread(m->verts[i].uv, sizeof(float), 2, file);
		for(int j = 0; j < 2; j++)
			m->verts[i].uv[j] = float_endian_flip(m->verts[i].uv[j]);
	}

	fread(&m->num_indis, sizeof(uint16_t), 1, file);
	m->num_indis = uint16_endian_flip(m->num_indis);
	m->indis = malloc(sizeof(uint16_t) * m->num_indis);
	for(int i = 0; i < m->num_indis; i++) {
		fread(m->indis + i, sizeof(uint16_t), 1, file);
		m->indis[i] = uint16_endian_flip(m->indis[i]);
	}

	fread(&m->tex_index, sizeof(uint16_t), 1, file);
	m->tex_index = uint16_endian_flip(m->tex_index);

	/* debugging
	printf("\tname=%s, num_verts=%d, num_indis=%d, tex_index=%d\n",
			m->name, m->num_verts, m->num_indis, m->tex_index);

	for(uint16_t i = 0; i < m->num_verts; i++) {
		vertex_t *v = m->verts + i;
		printf("\t\t%d: pos=(%f, %f, %f), uv=(%f, %f)\n", i,
				v->pos[0], v->pos[1], v->pos[2],
				v->uv[0], v->uv[1]);
	}

	printf("\n");

	for(uint16_t i = 0; i < m->num_indis / 3; i++)
		printf("\t\t%d\t%d\t%d\n",
				m->indis[i * 3 + 0],
				m->indis[i * 3 + 1],
				m->indis[i * 3 + 2]);

	printf("\n");
	*/
}

static void _anim_read(animation_t *a, FILE *file)
{
	fread(a->name, sizeof(char), NAME_MAX_LEN, file);
	fread(&a->mesh_index, sizeof(uint16_t), 1, file);
	a->mesh_index = uint16_endian_flip(a->mesh_index);
	fread(&a->length, sizeof(uint16_t), 1, file);
	a->length = uint16_endian_flip(a->length);
	fread(&a->num_pos, sizeof(uint16_t), 1, file);
	a->num_pos = uint16_endian_flip(a->num_pos);
	fread(&a->num_rot, sizeof(uint16_t), 1, file);
	a->num_rot = uint16_endian_flip(a->num_rot);
	fread(&a->num_sca, sizeof(uint16_t), 1, file);
	a->num_sca = uint16_endian_flip(a->num_sca);

	a->pos = malloc(sizeof(vec3_key_t) * a->num_pos);
	for(uint16_t i = 0; i < a->num_pos; i++) {
		fread(&a->pos[i].frame, sizeof(uint16_t), 1, file);
		a->pos[i].frame = uint16_endian_flip(a->pos[i].frame);
		fread(a->pos[i].vec, sizeof(float), 3, file);
		for(uint16_t j = 0; j < 3; j++)
			a->pos[i].vec[j] = float_endian_flip(a->pos[i].vec[j]);
	}

	a->rot = malloc(sizeof(vec4_key_t) * a->num_rot);
	for(uint16_t i = 0; i < a->num_rot; i++) {
		fread(&a->rot[i].frame, sizeof(uint16_t), 1, file);
		a->rot[i].frame = uint16_endian_flip(a->rot[i].frame);
		fread(a->rot[i].vec, sizeof(float), 4, file);
		for(uint16_t j = 0; j < 4; j++)
			a->rot[i].vec[j] = float_endian_flip(a->rot[i].vec[j]);
	}

	a->sca = malloc(sizeof(vec3_key_t) * a->num_sca);
	for(uint16_t i = 0; i < a->num_sca; i++) {
		fread(&a->sca[i].frame, sizeof(uint16_t), 1, file);
		a->sca[i].frame = uint16_endian_flip(a->sca[i].frame);
		fread(a->sca[i].vec, sizeof(float), 3, file);
		for(uint16_t j = 0; j < 3; j++)
			a->sca[i].vec[j] = float_endian_flip(a->sca[i].vec[j]);
	}

	/* debugging
	printf("\tname=%s, mesh_index=%d, length=%d, "
			"npos=%d, nrot=%d, nsca=%d\n",
			a->name, a->mesh_index, a->length,
			a->num_pos, a->num_rot, a->num_sca);

	for(uint16_t i = 0; i < a->num_pos; i++)
		printf("\t\tpos%d=(%f, %f, %f)\n", a->pos[i].frame,
				a->pos[i].vec[0], a->pos[i].vec[1],
				a->pos[i].vec[2]);

	printf("\n");

	for(uint16_t i = 0; i < a->num_rot; i++)
		printf("\t\trot%d=(%f, %f, %f, %f)\n", a->rot[i].frame,
				a->rot[i].vec[0], a->rot[i].vec[1],
				a->rot[i].vec[2], a->rot[i].vec[3]);

	printf("\n");

	for(uint16_t i = 0; i < a->num_sca; i++)
		printf("\t\tsca%d=(%f, %f, %f)\n", a->sca[i].frame,
				a->sca[i].vec[0], a->sca[i].vec[1],
				a->sca[i].vec[2]);

	printf("\n");
	*/
}

/*
static void _matrix_print(float *mat, int indents)
{
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			for(int k = 0; k < indents; k++)
				printf("\t");
			printf("%f ", mat[j * 4 + i]);
		}
		printf("\n");
	}
	printf("\n");
}
*/

static void _node_read(node_t *n, FILE *file, int depth)
{
	fread(n->name, sizeof(char), NAME_MAX_LEN, file);
	fread(&n->mesh_index, sizeof(uint16_t), 1, file);
	n->mesh_index = uint16_endian_flip(n->mesh_index);
	fread(n->mat, sizeof(float), 4 * 4, file);
	for(int i = 0; i < 16; i++)
		n->mat[i] = float_endian_flip(n->mat[i]);

	fread(&n->num_children, sizeof(uint16_t), 1, file);
	n->num_children = uint16_endian_flip(n->num_children);
	/*
	for(int i = 0; i < depth; i++)
		printf("\t");
	printf("name=%s, mesh_index=%d num_children=%d\n",
			n->name, n->mesh_index, n->num_children);
	_matrix_print(n->mat, depth);
	*/

	n->children = malloc(sizeof(node_t) * n->num_children);
	for(int i = 0; i < n->num_children; i++)
		_node_read(n->children + i, file, depth + 1);
}

static void _scene_read_test(const char *path_out)
{
	FILE *file = fopen(path_out, "rb");
	if(!file) {
		fprintf(stderr, "Failed to read file from '%s'\n", path_out);
		exit(1);
	}

	scene_t scene;

	fread(&scene.num_meshes, sizeof(uint16_t), 1, file);
	scene.num_meshes = uint16_endian_flip(scene.num_meshes);
	scene.meshes = malloc(sizeof(mesh_t) * scene.num_meshes);
	// printf("num_meshes=%d\n", scene.num_meshes);
	for(int i = 0; i < scene.num_meshes; i++)
		_mesh_read(scene.meshes + i, file);

	fread(&scene.num_anims, sizeof(uint16_t), 1, file);
	scene.num_anims = uint16_endian_flip(scene.num_anims);
	scene.anims = malloc(sizeof(animation_t) * scene.num_anims);
	// printf("num_anims=%d\n", scene.num_anims);
	for(int i = 0; i < scene.num_anims; i++)
		_anim_read(scene.anims + i, file);

	_node_read(&scene.root_node, file, 0);

	fread(&scene.num_tex_paths, sizeof(uint16_t), 1, file);
	scene.num_tex_paths = uint16_endian_flip(scene.num_tex_paths);
	// printf("num_tex_paths=%d\n", scene.num_tex_paths);
	for(uint16_t i = 0; i < scene.num_tex_paths; i++) {
		fread(scene.tex_paths[i], sizeof(char), TEX_PATH_MAX_LEN, file);
		// printf("\ttex_path%d='%s'\n", i, scene.tex_paths[i]);
	}

	fclose(file);
}

static void _scene_flush(scene_t *s)
{
	s->num_meshes = 0;
	s->num_anims = 0;
	free(s->meshes);
	free(s->anims);
}

int main(int argc, char **argv)
{
	if(argc != 3)
		_print_usage(argv[0]);

	const char *path_in = argv[1];
	const struct aiScene *scenein = aiImportFile(path_in,
			aiProcess_JoinIdenticalVertices | aiProcess_Triangulate
			| aiProcess_ImproveCacheLocality
			| aiProcess_RemoveRedundantMaterials);
	if(!scenein) {
		fprintf(stderr, "Assimp Error (%s): %s\n", path_in,
				aiGetErrorString());
		exit(1);
	}

	const char *path_out = argv[2];
	scene_t sceneout;
	_scene_process(&sceneout, scenein);
	_scene_write(&sceneout, path_out);
	_scene_flush(&sceneout);
	_scene_read_test(path_out);

	return 0;
}
