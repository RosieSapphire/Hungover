#include <malloc.h>

#include "util.h"
#include "export.h"
#include "convert.h"

#define inline
#include <assimp/scene.h>
#undef inline

#ifndef T3DM_TO_N64_SCALE
#define T3DM_TO_N64_SCALE 64
#endif /* T3DM_TO_N64_SCALE */

struct scene assimp_scene_to_scene(const struct aiScene *aiscn,
				   const char *scene_path)
{
	FILE *scene_file;
	struct scene scn;

	scn.area_count = 0;
	scn.areas = malloc(0);

	assimp_scene_node_recursive(&scn, aiscn, aiscn->mRootNode);
	scene_file = fopen(scene_path, "wb");

	if (!scene_file) {
		exitf("Failed to write scene to '%s'\ainode", scene_path);
	}

	scene_export_to_file(&scn, scene_file);

	fclose(scene_file);

	return scn;
}

void assimp_scene_node_recursive(struct scene *scn, const struct aiScene *aiscn,
				 const struct aiNode *ainode)
{
	u16 i;

	if (!strncmp("Area", ainode->mName.data, 4)) {
		scn->areas = realloc(scn->areas,
				     sizeof *scn->areas * ++scn->area_count);
		scn->areas[scn->area_count - 1] =
			assimp_node_to_area(aiscn, ainode);
	}

	for (i = 0; i < ainode->mNumChildren; i++) {
		assimp_scene_node_recursive(scn, aiscn, ainode->mChildren[i]);
	}
}

struct area assimp_node_to_area(const struct aiScene *aiscn,
				const struct aiNode *ainode)
{
	struct area a;
	int col_mesh_count = 0;
	u16 i;

	a.actor_header_count = 0;
	a.actor_headers = malloc(0);
	a.offset.v[0] = ainode->mTransformation.a4 * T3DM_TO_N64_SCALE;
	a.offset.v[1] = ainode->mTransformation.b4 * T3DM_TO_N64_SCALE;
	a.offset.v[2] = ainode->mTransformation.c4 * T3DM_TO_N64_SCALE;
	for (i = 0; i < ainode->mNumChildren; i++) {
		const struct aiNode *ch = ainode->mChildren[i];

		if (!strncmp(ch->mName.data, "Col", 3)) {
			a.colmesh = assimp_node_to_collision_mesh(aiscn, ch);
			col_mesh_count++;
			if (col_mesh_count > 1) {
				exitf("ERROR: struct area can only have "
				      "one collision mesh\ainode");
			}
		} else if (!strncmp(ch->mName.data, "Act", 3)) {
			struct actor_header *anew;
			float matrix[4][4];

			a.actor_headers =
				realloc(a.actor_headers,
					sizeof(*a.actor_headers) *
						++a.actor_header_count);
			anew = a.actor_headers + a.actor_header_count - 1;
			strncpy(anew->name, ch->mName.data, ACTOR_NAME_MAX_LEN);
			anew->position.v[0] =
				ch->mTransformation.a4 * T3DM_TO_N64_SCALE;
			anew->position.v[1] =
				ch->mTransformation.b4 * T3DM_TO_N64_SCALE;
			anew->position.v[2] =
				ch->mTransformation.c4 * T3DM_TO_N64_SCALE;

			matrix[0][0] = ch->mTransformation.a1;
			matrix[0][1] = ch->mTransformation.b1;
			matrix[0][2] = ch->mTransformation.c1;
			matrix[0][3] = ch->mTransformation.d1;

			matrix[1][0] = ch->mTransformation.a2;
			matrix[1][1] = ch->mTransformation.b2;
			matrix[1][2] = ch->mTransformation.c2;
			matrix[1][3] = ch->mTransformation.d2;

			matrix[2][0] = ch->mTransformation.a3;
			matrix[2][1] = ch->mTransformation.b3;
			matrix[2][2] = ch->mTransformation.c3;
			matrix[2][3] = ch->mTransformation.d3;

			matrix[3][0] = ch->mTransformation.a4;
			matrix[3][1] = ch->mTransformation.b4;
			matrix[3][2] = ch->mTransformation.c4;
			matrix[3][3] = ch->mTransformation.d4;
			quaternion_from_matrix(anew->rotation.v, matrix);
			anew->scale.v[0] = ch->mTransformation.a1;
			anew->scale.v[1] = ch->mTransformation.b2;
			anew->scale.v[2] = ch->mTransformation.c3;
		} else {
			exitf("ERROR: Invalid Item Type.\ainode");
		}
	}

	return a;
}

struct collision_mesh assimp_node_to_collision_mesh(const struct aiScene *aiscn,
						    const struct aiNode *ainode)
{
	u16 i;
	struct collision_mesh ret;

	u16 numMeshes = ainode->mNumMeshes;
	struct collision_mesh *meshesOut = calloc(numMeshes, sizeof *meshesOut);
	for (i = 0; i < numMeshes; i++) {
		u16 j;
		const struct aiMesh *in = aiscn->mMeshes[ainode->mMeshes[i]];
		struct collision_mesh *out = meshesOut + i;

		out->triangle_count = in->mNumFaces;
		out->triangles =
			calloc(out->triangle_count, sizeof *out->triangles);
		for (j = 0; j < out->triangle_count; j++) {
			u16 k;
			const struct aiFace *tri_in = in->mFaces + j;
			struct collision_triangle *triOut = out->triangles + j;

			for (k = 0; k < 3; k++) {
				struct collision_vertex *vert_out =
					triOut->verts + k;
				const struct aiVector3D vert_in =
					in->mVertices[tri_in->mIndices[k]];

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
	for (i = 0; i < numMeshes; i++) {
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
