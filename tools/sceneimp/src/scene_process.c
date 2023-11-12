#include <assert.h>
#include <malloc.h>

#include "util.h"
#include "mesh.h"
#include "animation.h"
#include "scene.h"

/**
 * mesh_process - Process Mesh for Scene
 * @mo: Mesh Out
 * @mi: Mesh In
 */
void mesh_process(struct mesh *mo, const struct aiMesh *mi)
{
	strncpy(mo->name, mi->mName.data, mi->mName.length + 1);
	mo->num_verts = mi->mNumVertices;
	mo->verts = malloc(mo->num_verts * sizeof(struct vertex));
	for (u16 i = 0; i < mo->num_verts; i++)
	{
		struct vertex *v = mo->verts + i;

		memcpy(v->pos, &mi->mVertices[i].x, sizeof(f32) * 3);
		memcpy(v->uv, &mi->mTextureCoords[0][i], sizeof(f32) * 3);
	}

	mo->num_indis = mi->mNumFaces * 3;
	mo->indis = malloc(mo->num_indis * sizeof(u16));
	for (u16 i = 0; i < mi->mNumFaces; i++)
	{
		const struct aiFace *face = mi->mFaces + i;

		assert(face->mNumIndices == 3);
		for (u16 j = 0; j < face->mNumIndices; j++)
			mo->indis[i * 3 + j] = face->mIndices[j];
	}

	mo->tex_index = mi->mMaterialIndex;
}

/**
 * node_process - Process Node for Scene
 * @no: Node Out
 * @ni: Node In
 */
void node_process(struct node *no, const struct aiNode *ni)
{
	strncpy(no->name, ni->mName.data, ni->mName.length + 1);
	no->mesh_index = ni->mNumMeshes ? ni->mMeshes[0] : 0xFFFF;
	matrix_transpose((f32 *)&ni->mTransformation, no->mat);
	no->num_children = ni->mNumChildren;
	no->children = malloc(sizeof(struct node) * no->num_children);
	for (int i = 0; i < no->num_children; i++)
		node_process(no->children + i, ni->mChildren[i]);
}

/**
 * _anim_process_keyframes - Processes just the keyframes of an Animation
 * @ao: Animation Out
 * @ai: Animation In
 */
static void _anim_process_keyframes(struct animation *ao,
				    const struct aiAnimation *ai)
{
	ao->pos = malloc(sizeof(struct vec3_key) * ao->num_pos);
	for (u16 i = 0; i < ao->num_pos; i++)
	{
		ao->pos[i].frame = (u16)roundf(
				(ai->mChannels[0]->mPositionKeys[i].mTime
				 / ai->mTicksPerSecond) * 24);
		ao->pos[i].vec[0] = ai->mChannels[0]->mPositionKeys[i].mValue.x;
		ao->pos[i].vec[1] = ai->mChannels[0]->mPositionKeys[i].mValue.y;
		ao->pos[i].vec[2] = ai->mChannels[0]->mPositionKeys[i].mValue.z;
	}

	ao->rot = malloc(sizeof(struct vec4_key) * ao->num_rot);
	for (u16 i = 0; i < ao->num_rot; i++)
	{
		ao->rot[i].frame = (u16)roundf(
				(ai->mChannels[0]->mRotationKeys[i].mTime
				 / ai->mTicksPerSecond) * 24);
		ao->rot[i].vec[0] = ai->mChannels[0]->mRotationKeys[i].mValue.x;
		ao->rot[i].vec[1] = ai->mChannels[0]->mRotationKeys[i].mValue.y;
		ao->rot[i].vec[2] = ai->mChannels[0]->mRotationKeys[i].mValue.z;
		ao->rot[i].vec[3] = ai->mChannels[0]->mRotationKeys[i].mValue.w;
	}

	ao->sca = malloc(sizeof(struct vec3_key) * ao->num_sca);
	for (u16 i = 0; i < ao->num_sca; i++)
	{
		ao->sca[i].frame = (u16)roundf(
				(ai->mChannels[0]->mScalingKeys[i].mTime
				 / ai->mTicksPerSecond) * 24);
		ao->sca[i].vec[0] = ai->mChannels[0]->mScalingKeys[i].mValue.x;
		ao->sca[i].vec[1] = ai->mChannels[0]->mScalingKeys[i].mValue.y;
		ao->sca[i].vec[2] = ai->mChannels[0]->mScalingKeys[i].mValue.z;
	}
}

/**
 * anim_process - Processes an Animation
 * @ao: Animation Out
 * @so: Scene Out
 * @ai: Animation In
 */
void anim_process(struct animation *ao, const struct scene *so,
		const struct aiAnimation *ai)
{
	u16 nums[3] = {
		ao->num_pos = ai->mChannels[0]->mNumPositionKeys,
		ao->num_rot = ai->mChannels[0]->mNumRotationKeys,
		ao->num_sca = ai->mChannels[0]->mNumScalingKeys,
	};
	u16 max = 0;

	/* acting */
	strncpy(ao->name, ai->mName.data, ai->mName.length + 1);
	ao->mesh_index = mesh_index_from_name(ai->mChannels[0]->mNodeName.data,
				       so->meshes, so->num_meshes);

	for (int i = 0; i < 3; i++)
	{
		if (max < nums[i])
			max = nums[i];

		ao->length = max;
	}

	_anim_process_keyframes(ao, ai);
}

/**
 * scene_process - Processes a Scene
 * @so: Scene Out
 * @si: Scene In
 */
void scene_process(struct scene *so, const struct aiScene *si)
{
	so->num_meshes = si->mNumMeshes;
	so->meshes = malloc(so->num_meshes * sizeof(struct mesh));
	for (u16 i = 0; i < so->num_meshes; i++)
		mesh_process(so->meshes + i, si->mMeshes[i]);

	so->num_anims = si->mNumAnimations;
	so->anims = malloc(so->num_anims * sizeof(struct animation));
	for (u16 i = 0; i < so->num_anims; i++)
		anim_process(so->anims + i, so, si->mAnimations[i]);

	node_process(&so->root_node, si->mRootNode);

	so->num_tex_paths = si->mNumMaterials;
	for (int i = 0; i < so->num_tex_paths; i++)
	{
		const struct aiMaterial *mat = si->mMaterials[i];
		struct aiString tex_path;

		aiGetMaterialString(mat, AI_MATKEY_NAME, &tex_path);
		memcpy(so->tex_paths[i], tex_path.data, tex_path.length + 1);
	}
}
