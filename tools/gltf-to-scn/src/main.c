#include <assert.h>
#include <stdio.h>
#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <json-c/json.h>

#define T3DM_TO_N64_SCALE 64.f

#define IS_USING_GLTF_TO_SCN
#include "../../../include/engine/scene.h"
#include "../../../include/engine/actor_static.h"
#include "../../../include/engine/actor_door.h"
#include "../../../include/engine/actor_microwave.h"
#include "../../../include/engine/actor_pickup.h"

#include "endian.h"

#define NODE_NAME_MAX_LEN 64

enum { NODE_TYPE_AREA, NODE_TYPE_COLMESH, NODE_TYPE_ACTOR, NODE_TYPE_COUNT };

struct node {
	char name[NODE_NAME_MAX_LEN];
	unsigned int node_type;
	unsigned int area_index;
	unsigned int actor_type;
	unsigned int area_dest;
	unsigned int pickup_type;
	char model_path[ACTOR_STATIC_MDLPATH_MAX_LEN];
	float position[3];
	float scale[3];
	float rotation[4];
	unsigned int children_count;
	unsigned int *children_indices;
};

static const struct aiScene *scene_in = NULL;

u8 actor_static_count;
struct actor_static actor_statics[ACTOR_STATIC_MAX_COUNT];

u8 actor_door_count;
struct actor_door actor_doors[ACTOR_DOOR_MAX_COUNT];

u8 actor_microwave_count;
struct actor_microwave actor_microwaves[ACTOR_MICROWAVE_MAX_COUNT];

u8 actor_pickup_count;
struct actor_pickup actor_pickups[ACTOR_PICKUP_MAX_COUNT];

static unsigned int _node_type_enum_from_str(const char *str)
{
	if (!strcmp(str, "Area")) {
		return NODE_TYPE_AREA;
	}

	if (!strcmp(str, "ColMesh")) {
		return NODE_TYPE_COLMESH;
	}

	if (!strcmp(str, "Actor")) {
		return NODE_TYPE_ACTOR;
	}

	return -1;
}

static unsigned int _actor_type_enum_from_str(const char *str)
{
	if (!strcmp(str, "Static")) {
		return ACTOR_TYPE_STATIC;
	}

	if (!strcmp(str, "Door")) {
		return ACTOR_TYPE_DOOR;
	}

	if (!strcmp(str, "Microwave")) {
		return ACTOR_TYPE_MICROWAVE;
	}

	if (!strcmp(str, "Pickup")) {
		return ACTOR_TYPE_PICKUP;
	}

	return -1;
}

static void _depth_print_tabs(const unsigned int depth)
{
	for (unsigned int i = 0; i < depth; i++) {
		printf("\t");
	}
}

static const char *_actor_type_str_from_enum(const unsigned int enm)
{
	switch (enm) {
	case ACTOR_TYPE_STATIC:
		return "Static";

	case ACTOR_TYPE_DOOR:
		return "Door";

	case ACTOR_TYPE_MICROWAVE:
		return "Microwave";

	case ACTOR_TYPE_PICKUP:
		return "Pickup";
	}

	return NULL;
}

static const struct aiNode *_ainode_from_name(const struct aiNode *ainode,
					      const char *node_name)
{
	if (!strcmp(ainode->mName.data, node_name)) {
		return ainode;
	}

	for (unsigned int i = 0; i < ainode->mNumChildren; i++) {
		const struct aiNode *child = ainode->mChildren[i];
		const struct aiNode *found =
			_ainode_from_name(child, node_name);
		if (found) {
			return found;
		}
	}

	return NULL;
}

static void _quaternion_from_matrix(float quat[4], float matrix[4][4])
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

static void _node_actor_process(struct node *node, struct json_object *extras)
{
	struct json_object *js_node_actor_type;
	json_object_object_get_ex(extras, "ActorType", &js_node_actor_type);
	assert(js_node_actor_type);
	node->actor_type = _actor_type_enum_from_str(
		json_object_get_string(js_node_actor_type));

	switch (node->actor_type) {
	case ACTOR_TYPE_STATIC: {
		struct json_object *js_node_model_path;
		json_object_object_get_ex(extras, "ModelPath",
					  &js_node_model_path);
		assert(js_node_model_path);
		memset(node->model_path, 0, ACTOR_STATIC_MDLPATH_MAX_LEN);
		strncpy(node->model_path,
			json_object_get_string(js_node_model_path),
			ACTOR_STATIC_MDLPATH_MAX_LEN);
		printf("%s: %s\n", node->name, node->model_path);
		break;
	}

	case ACTOR_TYPE_DOOR: {
		struct json_object *js_node_area_dest;
		json_object_object_get_ex(extras, "AreaDest",
					  &js_node_area_dest);
		assert(js_node_area_dest);
		node->area_dest = json_object_get_int(js_node_area_dest);
		break;
	}

	case ACTOR_TYPE_MICROWAVE:
		break;

	case ACTOR_TYPE_PICKUP: {
		struct json_object *js_node_pickup_type;
		json_object_object_get_ex(extras, "PickupType",
					  &js_node_pickup_type);
		assert(js_node_pickup_type);
		node->pickup_type = json_object_get_int(js_node_pickup_type);
		break;
	}
	}

	const struct aiNode *ainode =
		_ainode_from_name(scene_in->mRootNode, node->name);

	node->position[0] = ainode->mTransformation.a4 * T3DM_TO_N64_SCALE;
	node->position[1] = ainode->mTransformation.b4 * T3DM_TO_N64_SCALE;
	node->position[2] = ainode->mTransformation.c4 * T3DM_TO_N64_SCALE;

	node->scale[0] = 1.f;
	node->scale[1] = 1.f;
	node->scale[2] = 1.f;

	float matrix[4][4];
	matrix[0][0] = ainode->mTransformation.a1;
	matrix[0][1] = ainode->mTransformation.b1;
	matrix[0][2] = ainode->mTransformation.c1;
	matrix[0][3] = ainode->mTransformation.d1;

	matrix[1][0] = ainode->mTransformation.a2;
	matrix[1][1] = ainode->mTransformation.b2;
	matrix[1][2] = ainode->mTransformation.c2;
	matrix[1][3] = ainode->mTransformation.d2;

	matrix[2][0] = ainode->mTransformation.a3;
	matrix[2][1] = ainode->mTransformation.b3;
	matrix[2][2] = ainode->mTransformation.c3;
	matrix[2][3] = ainode->mTransformation.d3;

	matrix[3][0] = ainode->mTransformation.a4;
	matrix[3][1] = ainode->mTransformation.b4;
	matrix[3][2] = ainode->mTransformation.c4;
	matrix[3][3] = ainode->mTransformation.d4;

	_quaternion_from_matrix(node->rotation, matrix);
}

const struct node **_node_children_get_actors(const struct node *node_array,
					      const struct node *node_cur,
					      unsigned short *actor_count)
{
	const struct node **children = malloc(0);

	for (unsigned int i = 0; i < node_cur->children_count; i++) {
		const struct node *cur =
			node_array + (node_cur->children_indices[i]);
		assert(cur);

		if (cur->node_type != NODE_TYPE_ACTOR) {
			continue;
		}

		children =
			realloc(children, sizeof(*children) * ++(*actor_count));
		children[(*actor_count) - 1] = cur;
	}

	return children;
}

const struct node *_node_children_get_colmesh(const struct node *node_array,
					      const struct node *node_cur)
{
	for (unsigned int i = 0; i < node_cur->children_count; i++) {
		const struct node *cur =
			node_array + (node_cur->children_indices[i]);
		assert(cur);

		if (cur->node_type == NODE_TYPE_COLMESH) {
			return cur;
		}
	}

	return NULL;
}

static void _triangle_calc_normal(struct collision_triangle *tri)
{
	float a[3], b[3], mag;

	a[0] = tri->verts[1].pos[0] - tri->verts[0].pos[0];
	a[1] = tri->verts[1].pos[1] - tri->verts[0].pos[1];
	a[2] = tri->verts[1].pos[2] - tri->verts[0].pos[2];

	b[0] = tri->verts[2].pos[0] - tri->verts[0].pos[0];
	b[1] = tri->verts[2].pos[1] - tri->verts[0].pos[1];
	b[2] = tri->verts[2].pos[2] - tri->verts[0].pos[2];

	tri->norm[0] = a[1] * b[2] - a[2] * b[1];
	tri->norm[1] = a[2] * b[0] - a[0] * b[2];
	tri->norm[2] = a[0] * b[1] - a[1] * b[0];

	mag = sqrtf(tri->norm[0] * tri->norm[0] + tri->norm[1] * tri->norm[1] +
		    tri->norm[2] * tri->norm[2]);

	if (!mag) {
		return;
	}

	tri->norm[0] /= mag;
	tri->norm[1] /= mag;
	tri->norm[2] /= mag;
}

static struct collision_mesh _node_to_colmesh(const struct aiNode *ainode)
{
	u16 mesh_count = ainode->mNumMeshes;
	struct collision_mesh *meshes_out =
		calloc(mesh_count, sizeof *meshes_out);
	struct collision_mesh ret;
	for (unsigned int i = 0; i < mesh_count; i++) {
		const struct aiMesh *in = scene_in->mMeshes[ainode->mMeshes[i]];
		struct collision_mesh *out = meshes_out + i;

		out->triangle_count = in->mNumFaces;
		out->triangles =
			calloc(out->triangle_count, sizeof *out->triangles);
		for (unsigned int j = 0; j < out->triangle_count; j++) {
			u16 k;
			const struct aiFace *tri_in = in->mFaces + j;
			struct collision_triangle *tri_out = out->triangles + j;

			for (k = 0; k < 3; k++) {
				struct collision_vertex *vert_out =
					tri_out->verts + k;
				const struct aiVector3D vert_in =
					in->mVertices[tri_in->mIndices[k]];

				vert_out->pos[0] =
					vert_in.x * T3DM_TO_N64_SCALE;
				vert_out->pos[1] =
					vert_in.y * T3DM_TO_N64_SCALE;
				vert_out->pos[2] =
					vert_in.z * T3DM_TO_N64_SCALE;
			}
			_triangle_calc_normal(tri_out);
		}
	}

	ret.triangle_count = 0;
	ret.triangles = malloc(0);
	for (unsigned int i = 0; i < mesh_count; i++) {
		const struct collision_mesh *m = meshes_out + i;

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

static void _node_to_area(struct area *area, const struct node *node_array,
			  const struct node *node_cur)
{
	strncpy(area->name, node_cur->name, AREA_NAME_MAX_LEN);

	const struct aiNode *ainode =
		_ainode_from_name(scene_in->mRootNode, node_cur->name);
	area->offset.v[0] = ainode->mTransformation.a4 * T3DM_TO_N64_SCALE;
	area->offset.v[1] = ainode->mTransformation.b4 * T3DM_TO_N64_SCALE;
	area->offset.v[2] = ainode->mTransformation.c4 * T3DM_TO_N64_SCALE;

	const struct node *colmesh_node =
		_node_children_get_colmesh(node_array, node_cur);
	const struct aiNode *aich =
		_ainode_from_name(scene_in->mRootNode, colmesh_node->name);
	area->colmesh = _node_to_colmesh(aich);

	area->actor_header_count = 0;

	const struct node **node_actors = _node_children_get_actors(
		node_array, node_cur, &area->actor_header_count);
	assert(node_actors);
	area->actor_headers =
		calloc(area->actor_header_count, sizeof(*area->actor_headers));
	assert(area->actor_headers);

	for (unsigned int i = 0; i < area->actor_header_count; i++) {
		struct actor_header *actor_cur = area->actor_headers + i;
		assert(actor_cur);

		const struct node *node_cur = node_actors[i];
		assert(node_cur);

		strncpy(actor_cur->name, node_cur->name, ACTOR_NAME_MAX_LEN);
		actor_cur->type = node_cur->actor_type;

		/* ALLOCATE OBJECTS */
		switch (actor_cur->type) {
		case ACTOR_TYPE_STATIC: {
			struct actor_static *stat =
				actor_statics + actor_static_count++;
			assert(stat);
			actor_cur->type_index = actor_static_count - 1;
			memset(stat->mdl_path, 0, ACTOR_STATIC_MDLPATH_MAX_LEN);
			strncpy(stat->mdl_path, node_cur->model_path,
				ACTOR_STATIC_MAX_COUNT);
			break;
		}
		case ACTOR_TYPE_DOOR: {
			struct actor_door *door =
				actor_doors + actor_door_count++;
			assert(door);
			actor_cur->type_index = actor_door_count - 1;
			door->area_dest = node_cur->area_dest;
			break;
		}
		case ACTOR_TYPE_MICROWAVE: {
			struct actor_microwave *mic =
				actor_microwaves + actor_microwave_count++;
			assert(mic);
			actor_cur->type_index = actor_microwave_count - 1;
			break;
		}

		case ACTOR_TYPE_PICKUP: {
			struct actor_pickup *pu =
				actor_pickups + actor_pickup_count++;
			assert(pu);
			actor_cur->type_index = actor_pickup_count - 1;
			pu->type = node_cur->pickup_type;
			break;
		}
		}

		actor_cur->position.v[0] = node_cur->position[0];
		actor_cur->position.v[1] = node_cur->position[1];
		actor_cur->position.v[2] = node_cur->position[2];
		actor_cur->scale.v[0] = node_cur->scale[0];
		actor_cur->scale.v[1] = node_cur->scale[1];
		actor_cur->scale.v[2] = node_cur->scale[2];
		actor_cur->rotation.v[0] = node_cur->rotation[0];
		actor_cur->rotation.v[1] = node_cur->rotation[1];
		actor_cur->rotation.v[2] = node_cur->rotation[2];
		actor_cur->rotation.v[3] = node_cur->rotation[3];
	}

	free(node_actors);
}

static void _node_to_scene(struct scene *scn, const struct node *node_array,
			   const struct node *node_cur)
{
	if (node_cur->node_type == NODE_TYPE_AREA) {
		scn->areas = realloc(scn->areas,
				     sizeof(*scn->areas) * ++scn->area_count);

		struct area *area = scn->areas + scn->area_count - 1;
		_node_to_area(area, node_array, node_cur);
	}
}

static void _scene_debug(const struct scene *scn)
{
	printf("Area Count: %d\n", scn->area_count);
	for (unsigned int i = 0; i < scn->area_count; i++) {
		struct area *area = scn->areas + i;
		_depth_print_tabs(1);
		printf("Area %d '%s': (%d)\n", i, area->name,
		       area->actor_header_count);
		_depth_print_tabs(1);
		printf(" - Offset: (%f, %f, %f)\n", area->offset.v[0],
		       area->offset.v[1], area->offset.v[2]);
		_depth_print_tabs(2);
		printf("Collision Mesh:\n");
		_depth_print_tabs(2);
		printf(" - Offset: (%f, %f, %f)\n", area->colmesh.offset.v[0],
		       area->colmesh.offset.v[1], area->colmesh.offset.v[2]);
		_depth_print_tabs(2);
		printf(" - Triangle Count: %d\n", area->colmesh.triangle_count);
#if 0
		for (unsigned int j = 0; j < area->colmesh.triangle_count;
		     j++) {
			const struct collision_triangle *tri =
				area->colmesh.triangles + j;
			_depth_print_tabs(2);
			printf(" - Triangle %d:\n", j);

			for (unsigned int k = 0; k < 3; k++) {
				_depth_print_tabs(2);
				printf(" -   P%d: (%f, %f, %f)\n", k + 1,
				       tri->verts[k].pos[0],
				       tri->verts[k].pos[1],
				       tri->verts[k].pos[2]);
			}
			_depth_print_tabs(2);

			printf(" -    N: (%f, %f, %f)\n", tri->norm[0],
			       tri->norm[1], tri->norm[2]);
		}
#endif

		for (unsigned int j = 0; j < area->actor_header_count; j++) {
			struct actor_header *actor = area->actor_headers + j;
			_depth_print_tabs(2);
			printf("Actor %d '%s':\n", j, actor->name);
			_depth_print_tabs(2);
			printf(" - Pos: %f, %f, %f\n", actor->position.v[0],
			       actor->position.v[1], actor->position.v[2]);
			_depth_print_tabs(2);
			printf(" - Sca: %f, %f, %f\n", actor->scale.v[0],
			       actor->scale.v[1], actor->scale.v[2]);
			_depth_print_tabs(2);
			printf(" - Rot: %f, %f, %f, %f\n", actor->rotation.v[0],
			       actor->rotation.v[1], actor->rotation.v[2],
			       actor->rotation.v[3]);
			_depth_print_tabs(2);
			printf(" - Type: %s\n",
			       _actor_type_str_from_enum(actor->type));
			_depth_print_tabs(2);
			printf(" - Type Index: %d\n", actor->type_index);
			switch (actor->type) {
			case ACTOR_TYPE_STATIC: {
				struct actor_static *stat =
					actor_statics + actor->type_index;
				_depth_print_tabs(2);
				printf(" - ModelPath: '%s'\n", stat->mdl_path);
				break;
			}

			case ACTOR_TYPE_DOOR: {
				struct actor_door *door =
					actor_doors + actor->type_index;
				_depth_print_tabs(2);
				printf(" - AreaDest: %d\n", door->area_dest);
				break;
			}

			case ACTOR_TYPE_MICROWAVE:
				// TODO: IMPLEMENT
				break;

			case ACTOR_TYPE_PICKUP: {
				struct actor_pickup *pu =
					actor_pickups + actor->type_index;
				_depth_print_tabs(2);
				printf(" - PickupType: %d\n", pu->type);
				break;
			}
			}
		}
	}
}

static void _scene_export(const char *path_out, const struct scene *scn)
{
	FILE *file = fopen(path_out, "wb");
	assert(file);

	fwrite_ef16(&scn->area_count, file);
	for (unsigned int i = 0; i < scn->area_count; i++) {
		const struct area *area = scn->areas + i;
		for (unsigned int j = 0; j < 3; j++) {
			fwrite_ef32(area->offset.v + j, file);
		}

		const struct collision_mesh *cm = &area->colmesh;
		for (unsigned int j = 0; j < 3; j++) {
			fwrite_ef32(cm->offset.v + j, file);
		}

		fwrite_ef16(&cm->triangle_count, file);
		for (unsigned int j = 0; j < cm->triangle_count; j++) {
			const struct collision_triangle *tri =
				cm->triangles + j;
			for (unsigned int k = 0; k < 3; k++) {
				for (unsigned int l = 0; l < 3; l++) {
					fwrite_ef32(tri->verts[k].pos + l,
						    file);
				}
			}

			for (unsigned int k = 0; k < 3; k++) {
				fwrite_ef32(tri->norm + k, file);
			}
		}

		fwrite_ef16(&area->actor_header_count, file);
		for (unsigned int j = 0; j < area->actor_header_count; j++) {
			struct actor_header *actor = area->actor_headers + j;
			fwrite(&actor->type, 1, 1, file);
			fwrite(&actor->type_index, 1, 1, file);

			switch (actor->type) {
			case ACTOR_TYPE_STATIC: {
				const struct actor_static *stat =
					actor_statics + actor->type_index;
				fwrite(&stat->mdl_path, 1,
				       ACTOR_STATIC_MDLPATH_MAX_LEN, file);
				break;
			}

			case ACTOR_TYPE_DOOR: {
				const struct actor_door *door =
					actor_doors + actor->type_index;
				fwrite_ef16(&door->area_dest, file);
				break;
			}

			case ACTOR_TYPE_MICROWAVE:
				break;

			case ACTOR_TYPE_PICKUP: {
				const struct actor_pickup *pu =
					actor_pickups + actor->type_index;
				fwrite(&pu->type, 1, 1, file);
				break;
			}
			}

			for (unsigned int k = 0; k < 3; k++) {
				fwrite_ef32(actor->position.v + k, file);
			}

			for (unsigned int k = 0; k < 3; k++) {
				fwrite_ef32(actor->scale.v + k, file);
			}

			for (unsigned int k = 0; k < 4; k++) {
				fwrite_ef32(actor->rotation.v + k, file);
			}
		}
	}

	fclose(file);
}

/*
static void _scene_import(const char *path, struct scene *scn)
{
	FILE *file = fopen(path, "rb");
	assert(file);

	fread_ef16(&scn->area_count, file);
	scn->areas = calloc(scn->area_count, sizeof(*scn->areas));
	for (unsigned int i = 0; i < scn->area_count; i++) {
		struct area *area = scn->areas + i;
		for (unsigned int j = 0; j < 3; j++) {
			fread_ef32(area->offset.v + j, file);
		}

		struct collision_mesh *cm = &area->colmesh;
		for (unsigned int j = 0; j < 3; j++) {
			fread_ef32(cm->offset.v + j, file);
		}

		fread_ef16(&cm->triangle_count, file);
		cm->triangles =
			calloc(cm->triangle_count, sizeof(*cm->triangles));
		for (unsigned int j = 0; j < cm->triangle_count; j++) {
			struct collision_triangle *tri = cm->triangles + j;
			for (unsigned int k = 0; k < 3; k++) {
				for (unsigned int l = 0; l < 3; l++) {
					fread_ef32(tri->verts[k].pos + l, file);
				}
			}

			for (unsigned int k = 0; k < 3; k++) {
				fread_ef32(tri->norm + k, file);
			}
		}

		fread_ef16(&area->actor_header_count, file);
		area->actor_headers = calloc(area->actor_header_count,
					     sizeof(*area->actor_headers));
		for (unsigned int j = 0; j < area->actor_header_count; j++) {
			struct actor_header *actor = area->actor_headers + j;
			fread(&actor->type, 1, 1, file);
			fread(&actor->type_index, 1, 1, file);

			switch (actor->type) {
			case ACTOR_TYPE_STATIC:
				break;

			case ACTOR_TYPE_DOOR: {
				struct actor_door *door =
					actor_doors + actor->type_index;
				fread_ef16(&door->area_dest, file);
				break;
			}

			case ACTOR_TYPE_MICROWAVE:
				break;
			}

			for (unsigned int k = 0; k < 3; k++) {
				fread_ef32(actor->position.v + k, file);
			}

			for (unsigned int k = 0; k < 3; k++) {
				fread_ef32(actor->scale.v + k, file);
			}

			for (unsigned int k = 0; k < 4; k++) {
				fread_ef32(actor->rotation.v + k, file);
			}
		}
	}

	fclose(file);
}
*/

static void _scene_free(struct scene *scn)
{
	for (unsigned int i = 0; i < scn->area_count; i++) {
		struct area *area = scn->areas + i;
		area->colmesh.offset = (T3DVec3){ { 0, 0, 0 } };
		free(area->colmesh.triangles);
		area->colmesh.triangle_count = 0;

		memset(area->name, 0, AREA_NAME_MAX_LEN);
		area->offset = (T3DVec3){ { 0, 0, 0 } };

		free(area->actor_headers);
		area->actor_header_count = 0;
	}

	free(scn->areas);
	scn->area_count = 0;
}

int main(int argc, char **argv)
{
	assert(argc == 3);

	const char *path_in = argv[1];
	const char *path_out = argv[2];

	// printf("path_in='%s' path_out='%s'\n", path_in, path_out);

	scene_in = aiImportFile(path_in, aiProcess_Triangulate);
	assert(scene_in);

	FILE *gltf_file = fopen(path_in, "rb");
	assert(gltf_file);

	fseek(gltf_file, 0, SEEK_END);

	unsigned int gltf_file_size = ftell(gltf_file);
	rewind(gltf_file);

	char *gltf_buf = calloc(gltf_file_size, 1);
	fread(gltf_buf, 1, gltf_file_size, gltf_file);
	fclose(gltf_file);

	struct json_object *json_parsed = json_tokener_parse(gltf_buf);
	assert(json_parsed);
	free(gltf_buf);

	struct json_object *js_nodes;
	json_object_object_get_ex(json_parsed, "nodes", &js_nodes);
	assert(js_nodes);

	unsigned int js_nodes_len = json_object_array_length(js_nodes);
	assert(js_nodes_len);

	struct node *nodes = calloc(js_nodes_len, sizeof(*nodes));
	assert(nodes);

	for (unsigned int i = 0; i < js_nodes_len; i++) {
		struct json_object *js_node =
			json_object_array_get_idx(js_nodes, i);
		assert(js_nodes);

		struct node *node = nodes + i;
		assert(node);

		struct json_object *js_node_name;
		json_object_object_get_ex(js_node, "name", &js_node_name);
		assert(js_node_name);
		strncpy(node->name, json_object_get_string(js_node_name),
			NODE_NAME_MAX_LEN);

		struct json_object *js_node_extras;
		json_object_object_get_ex(js_node, "extras", &js_node_extras);
		assert(js_node_extras);

		struct json_object *js_node_type;
		json_object_object_get_ex(js_node_extras, "NodeType",
					  &js_node_type);
		assert(js_node_type);

		node->node_type = _node_type_enum_from_str(
			json_object_get_string(js_node_type));

		switch (node->node_type) {
		case NODE_TYPE_AREA: {
			struct json_object *js_node_area_ind;
			json_object_object_get_ex(js_node_extras, "AreaIndex",
						  &js_node_area_ind);
			assert(js_node_area_ind);
			node->area_index =
				json_object_get_int(js_node_area_ind);
			break;
		}

		case NODE_TYPE_COLMESH: {
			/* TODO: Rip the model data from assimp! */
			break;
		}

		case NODE_TYPE_ACTOR:
			_node_actor_process(node, js_node_extras);
			break;
		}

		struct json_object *js_node_children;
		json_object_object_get_ex(js_node, "children",
					  &js_node_children);
		if (js_node_children) {
			unsigned int js_node_children_len =
				json_object_array_length(js_node_children);
			assert(js_node_children_len);

			node->children_count = js_node_children_len;
			node->children_indices =
				calloc(node->children_count,
				       sizeof(*node->children_indices));

			for (unsigned int j = 0; j < js_node_children_len;
			     j++) {
				struct json_object *js_node_child =
					json_object_array_get_idx(
						js_node_children, j);
				assert(js_node_child);

				unsigned int js_node_child_idx =
					json_object_get_int(js_node_child);
				unsigned int *node_child =
					node->children_indices + j;
				assert(node_child);
				*node_child = js_node_child_idx;
			}
		} else {
			node->children_count = 0;
			node->children_indices = NULL;
		}
	}

	struct json_object *js_scenes;
	json_object_object_get_ex(json_parsed, "scenes", &js_scenes);
	assert(js_scenes);

	unsigned int js_scenes_len = json_object_array_length(js_scenes);
	assert(js_scenes_len == 1);

	struct json_object *js_root = json_object_array_get_idx(js_scenes, 0);
	assert(js_root);

	struct json_object *js_root_name;
	json_object_object_get_ex(js_root, "name", &js_root_name);
	assert(js_root_name);

	struct json_object *js_root_nodes;
	json_object_object_get_ex(js_root, "nodes", &js_root_nodes);
	assert(js_root_nodes);

	unsigned int js_root_nodes_len =
		json_object_array_length(js_root_nodes);
	assert(js_root_nodes_len);

	struct node **root_nodes =
		calloc(js_root_nodes_len, sizeof(**root_nodes));

	struct scene scene_out;
	scene_out.area_count = 0;
	scene_out.areas = malloc(0);

	for (unsigned int i = 0; i < js_root_nodes_len; i++) {
		struct json_object *js_root_node =
			json_object_array_get_idx(js_root_nodes, i);
		assert(js_root_node);

		unsigned int js_root_node_idx =
			json_object_get_int(js_root_node);
		assert(js_root_node_idx);

		root_nodes[i] = nodes + js_root_node_idx;
		assert(root_nodes[i]);
		_node_to_scene(&scene_out, nodes, root_nodes[i]);
	}

	free(root_nodes);
	free(nodes);

	_scene_debug(&scene_out);
	_scene_export(path_out, &scene_out);
	_scene_free(&scene_out);

	/*
	_scene_import(path_out, &scene_out);
	_scene_debug(&scene_out);
	_scene_free(&scene_out);
	*/

	return 0;
}
