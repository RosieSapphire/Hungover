#include <stdio.h>
#include <stdlib.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "util.h"
#include "error.h"
#include "mesh.h"
#include "model.h"

// #define MODEL_DEBUG

static void _model_process_mesh(const struct aiMesh *msh_in, mesh_t *msh_out)
{
	msh_out->num_vertices = msh_in->mNumVertices;
	msh_out->num_indices = msh_in->mNumFaces * 3;
#ifdef MODEL_DEBUG
	printf("Mesh '%s' (%d Vtx, %d Ind):\n", msh_in->mName.data,
	       msh_out->num_vertices, msh_out->num_indices);
#endif

	strncpy(msh_out->name, msh_in->mName.data, msh_in->mName.length);
	msh_out->vertices =
		calloc(msh_out->num_vertices, sizeof *msh_out->vertices);
	for (unsigned int i = 0; i < msh_out->num_vertices; i++) {
		const struct aiVector3D pos = msh_in->mVertices[i];
		const struct aiVector3D norm = msh_in->mNormals[i];
		const struct aiVector3D uv = msh_in->mTextureCoords[0][i];
		const struct aiColor4D col = msh_in->mColors[0][i];
		vertex_t *vtx_out = msh_out->vertices + i;
		vtx_out->position[0] = pos.x;
		vtx_out->position[1] = pos.y;
		vtx_out->position[2] = pos.z;
		vtx_out->normal[0] = norm.x;
		vtx_out->normal[1] = norm.y;
		vtx_out->normal[2] = norm.z;
		vtx_out->texcoord[0] = uv.x;
		vtx_out->texcoord[1] = uv.y;
		vtx_out->color[0] = col.r;
		vtx_out->color[1] = col.g;
		vtx_out->color[2] = col.b;
		vtx_out->color[3] = col.a;
#ifdef MODEL_DEBUG
		printf("\tPos: (%f, %f, %f)   Norm: (%f, %f, %f)\n"
		       "UV: (%f, %f)   Col: (%f, %f, %f, %f)\n",
		       vtx_out->position[0], vtx_out->position[1],
		       vtx_out->position[2],

		       vtx_out->normal[0], vtx_out->normal[1],
		       vtx_out->normal[2],

		       vtx_out->texcoord[0], vtx_out->texcoord[1],
		       vtx_out->color[0], vtx_out->color[1], vtx_out->color[2],
		       vtx_out->color[3]);
#endif
	}
#ifdef MODEL_DEBUG
	printf("\n");
#endif

	msh_out->indices =
		calloc(msh_out->num_indices, sizeof *msh_out->indices);
	for (unsigned int i = 0; i < msh_in->mNumFaces; i++) {
		const struct aiFace face = msh_in->mFaces[i];
		for (unsigned int j = 0; j < 3; j++) {
			msh_out->indices[i * 3 + j] = face.mIndices[j];
		}
#ifdef MODEL_DEBUG
		printf("\tTri %d: (%d, %d, %d)", i, msh_out->indices[i * 3 + 0],
		       msh_out->indices[i * 3 + 1],
		       msh_out->indices[i * 3 + 2]);
		if (i && !(i % 3)) {
			printf("\n");
		}
#endif
	}
#ifdef MODEL_DEBUG
	printf("\n");
#endif
}

model_t model_create_from_file(const char *path, uint8_t flags,
			       uint16_t *num_textures, texture_t *textures)
{
	model_t mdl;

	const struct aiScene *aiscn = aiImportFile(path, aiProcess_Triangulate);
	if (!aiscn) {
		error_log("Assimp failed to load scene from '%s'\n", path);
	}

	/* models */
	strncpy(mdl.path, path, MODEL_PATH_MAX_LEN);
	mdl.flags = flags;
	mdl.num_meshes = aiscn->mNumMeshes;
#ifdef MODEL_DEBUG
	printf("Num Meshes: %d\n", mdl.num_meshes);
#endif
	mdl.meshes = calloc(mdl.num_meshes, sizeof *mdl.meshes);
	for (uint32_t i = 0; i < mdl.num_meshes; i++) {
		mesh_t *msh = mdl.meshes + i;
		_model_process_mesh(aiscn->mMeshes[i], msh);
		mesh_setup_buffers(msh);
		mesh_generate_aabb(msh);
	}

	glm_vec3_zero(mdl.position);

	/* textures */
	const uint16_t num_tex = aiscn->mNumMaterials - 1;
	if (!num_tex) {
		return mdl;
	}

	uint16_t num_textures_new = *num_textures;
	for (unsigned int i = *num_textures; i < *num_textures + num_tex; i++) {
		const unsigned int ci = i - *num_textures;
		const struct aiMaterial *mat = aiscn->mMaterials[ci];
		struct aiString str;
		if (aiGetMaterialString(mat, AI_MATKEY_NAME, &str) !=
		    AI_SUCCESS) {
			fprintf(stderr, "Failed to get name from material\n");
			exit(EXIT_FAILURE);
		}
		texture_load_to_array(str.data, &num_textures_new, textures);
		for (unsigned int j = 0; j < mdl.num_meshes; j++) {
			const struct aiMesh *aimsh = aiscn->mMeshes[j];
			mesh_t *omsh = mdl.meshes + j;
			if (aimsh->mMaterialIndex == ci) {
				omsh->tex_ind = i;
			}
		}
	}
	*num_textures = num_textures_new;

	return mdl;
}

int models_are_identical(const model_t *a, const model_t *b)
{
	if (a->num_meshes != b->num_meshes) {
		return 0;
	}

	for (unsigned int i = 0; i < a->num_meshes; i++) {
		if (!meshes_are_identical(a->meshes + i, b->meshes + i)) {
			return 0;
		}
	}

	return 1;
}

void model_render(model_t *m, const shader_t *s, const float *proj_matrix,
		  const float *view_matrix, vec3 offset,
		  const int use_meshes_aabb, texture_t *textures)
{
	if (!(m->flags & MODEL_FLAG_IS_ACTIVE)) {
		return;
	}

	vec3 total_offset;
	glm_vec3_add(m->position, offset, total_offset);
	for (unsigned int i = 0; i < m->num_meshes; i++) {
		mesh_t *msh = m->meshes + i;

		msh->flags &= ~(MESH_FLAG_SHOW_AABB);
		if (use_meshes_aabb) {
			msh->flags |= MESH_FLAG_SHOW_AABB;
		}
		mesh_render(msh, s, proj_matrix, view_matrix, total_offset,
			    textures);
	}
}

void model_move(model_t *dst, model_t *src)
{
	dst->flags = src->flags;
	dst->num_meshes = src->num_meshes;
	dst->meshes = calloc(dst->num_meshes, sizeof *dst->meshes);
	strncpy(dst->path, src->path, MODEL_PATH_MAX_LEN);
	for (unsigned int i = 0; i < dst->num_meshes; i++) {
		mesh_move(dst->meshes + i, src->meshes + i);
	}
	glm_vec3_copy(src->position, dst->position);

	src->flags = src->num_meshes = 0;
	if (src->meshes) {
		free(src->meshes);
		src->meshes = NULL;
	}
	memset(src->path, 0, MODEL_PATH_MAX_LEN);
	src->num_meshes = 0;
	glm_vec3_zero(src->position);
}

void model_destroy(model_t *m)
{
	for (uint32_t i = 0; i < m->num_meshes; i++) {
		mesh_destroy(m->meshes + i);
	}
	m->num_meshes = 0;
	if (m->meshes) {
		free(m->meshes);
		m->meshes = NULL;
	}

	glm_vec3_zero(m->position);
	m->flags = 0;
	memset(m->path, 0, MODEL_PATH_MAX_LEN);
}
