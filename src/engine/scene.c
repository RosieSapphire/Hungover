#include <stdio.h>
#include <malloc.h>

#include "engine/scene.h"

enum scene_index scene_index = SCENE_TESTROOM;

static void _mesh_debug(const smesh_t *m)
{
	debugf("Mesh '%s':\n", m->name);
	debugf("\tVerts (%d):\n", m->num_verts);
	for(int i = 0; i < m->num_verts; i++) {
		const vertex_t vert = m->verts[i];
		debugf("\t%d\tpos=(%f, %f, %f) uv=(%f, %f)\n",
				i, vert.pos[0], vert.pos[1], vert.pos[2],
				vert.uv[0], vert.uv[1]);
	}

	debugf("\n\tFaces (%d):\n", m->num_indis / 3);
	for(int i = 0; i < m->num_indis / 3; i++) {
		for(int j = 0; j < 3; j++)
			debugf("\t%d", m->indis[i * 3 + j]);
		debugf("\n");
	}
}

static void _anim_debug(const animation_t *anim)
{
	debugf("Animation '%s' (%d) (%d pos %d rot %d scale)\n", anim->name,
			anim->length, anim->num_pos, anim->num_rot,
			anim->num_sca);

	for(uint16_t i = 0; i < anim->num_pos; i++) {
		vec3_key_t *po = anim->pos + i;
		debugf("\t%d\tpos=(%f, %f, %f)\n", po->frame,
				po->vec[0], po->vec[1], po->vec[2]);
	}

	debugf("\n");

	for(uint16_t i = 0; i < anim->num_rot; i++) {
		vec4_key_t *ro = anim->rot + i;
		debugf("\t%d\trot=(%f, %f, %f, %f)\n", ro->frame,
				ro->vec[0], ro->vec[1], ro->vec[2], ro->vec[3]);
	}

	debugf("\n");

	for(uint16_t i = 0; i < anim->num_sca; i++) {
		vec3_key_t *so = anim->sca + i;
		debugf("\t%d\tsca=(%f, %f, %f)\n", so->frame,
				so->vec[0], so->vec[1], so->vec[2]);
	}
}

static void _node_debug(const node_t *n, int depth)
{
	for(int i = 0; i < depth; i++)
		debugf("\t");

	debugf("%s (mesh %d) (%d chils)\n", n->name,
			(int16_t)n->mesh_index, n->num_children);

	for(int i = 0; i < n->num_children; i++)
		_node_debug(n->children + i, depth + 1);
}

static void _scene_debug(const scene_t *s)
{
	for(int i = 0; i < s->num_meshes; i++)
		_mesh_debug(s->meshes + i);

	debugf("\n");

	for(int i = 0; i < s->num_anims; i++)
		_anim_debug(s->anims + i);

	debugf("\n");

	_node_debug(&s->root_node, 0);
}

static void _node_import(scene_t *s, node_t *n, FILE *file)
{
	fread(n->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	fread(&n->mesh_index, sizeof(uint16_t), 1, file);
	fread(&n->num_children, sizeof(uint16_t), 1, file);
	n->children = malloc(sizeof(node_t) * n->num_children);
	for(int i = 0; i < n->num_children; i++)
		_node_import(s, n->children + i, file);
}

scene_t *scene_load(const char *path)
{
	FILE *file = fopen(path, "rb");
	scene_t *scene = malloc(sizeof(scene_t));
	fread(&scene->num_meshes, sizeof(uint16_t), 1, file);
	scene->meshes = malloc(sizeof(smesh_t) * scene->num_meshes);
	for(int i = 0; i < scene->num_meshes; i++) {
		smesh_t *mesh = scene->meshes + i;
		fread(&mesh->name, sizeof(char), CONF_NAME_MAX_LEN, file);

		fread(&mesh->num_verts, sizeof(uint16_t), 1, file);
		mesh->verts = malloc(sizeof(vertex_t) * mesh->num_verts);
		fread(mesh->verts, sizeof(vertex_t), mesh->num_verts, file);

		fread(&mesh->num_indis, sizeof(uint16_t), 1, file);
		mesh->indis = malloc(sizeof(uint16_t) * mesh->num_indis);
		fread(mesh->indis, sizeof(uint16_t), mesh->num_indis, file);
	}

	fread(&scene->num_anims, sizeof(uint16_t), 1, file);
	scene->anims = malloc(sizeof(animation_t) * scene->num_anims);
	for(int i = 0; i < scene->num_anims; i++) {
		animation_t *anim = scene->anims + i;
		fread(&anim->name, sizeof(char), CONF_NAME_MAX_LEN, file);
		fread(&anim->mesh_index, sizeof(uint16_t), 1, file);
		fread(&anim->length, sizeof(uint16_t), 1, file);
		fread(&anim->num_pos, sizeof(uint16_t), 1, file);
		fread(&anim->num_rot, sizeof(uint16_t), 1, file);
		fread(&anim->num_sca, sizeof(uint16_t), 1, file);
		anim->frame_last = anim->frame = 0;
		anim->is_playing = true;

		anim->pos = malloc(sizeof(vec3_key_t) * anim->num_pos);
		for(int j = 0; j < anim->num_pos; j++) {
			fread(&anim->pos[j].frame, sizeof(uint16_t), 1, file);
			fread(anim->pos[j].vec, sizeof(float), 3, file);
		}

		anim->rot = malloc(sizeof(vec4_key_t) * anim->num_rot);
		for(int j = 0; j < anim->num_rot; j++) {
			fread(&anim->rot[j].frame, sizeof(uint16_t), 1, file);
			fread(anim->rot[j].vec, sizeof(float), 4, file);
		}

		anim->sca = malloc(sizeof(vec3_key_t) * anim->num_sca);
		for(int j = 0; j < anim->num_sca; j++) {
			fread(&anim->sca[j].frame, sizeof(uint16_t), 1, file);
			fread(anim->sca[j].vec, sizeof(float), 3, file);
		}
	}

	_node_import(scene, &scene->root_node, file);

	fclose(file);
	_scene_debug(scene);

	return scene;
}

void scene_unload(scene_t *s)
{
	for(int i = 0; i < s->num_meshes; i++)
		smesh_destroy(s->meshes);

	free(s);
}

static uint16_t _scene_anim_index_from_mesh_index(
		const scene_t *s, uint16_t mesh_index)
{
	if(mesh_index == 0xFFFF)
		return mesh_index;

	for(int i = 0; i < s->num_anims; i++) {
		int anim_ind = s->anims[i].mesh_index;
		if(mesh_index == anim_ind)
			return anim_ind;
	}

	return 0xFFFF;
}

static void _scene_node_mesh_draw(const scene_t *s, float subtick,
		const node_t *n, const uint32_t tid)
{
	int anim_index = _scene_anim_index_from_mesh_index(s, n->mesh_index);
	if(n->mesh_index != 0xFFFF) {
		if(anim_index != 0xFFFF) {
			glPushMatrix();
			animation_setup_matrix(s->anims + anim_index, subtick);
			smesh_draw(s->meshes + (n->mesh_index), tid);
			glPopMatrix();
		} else {
			smesh_draw(s->meshes + (n->mesh_index), tid);
		}
	}

	for(uint16_t i = 0; i < n->num_children; i++)
		_scene_node_mesh_draw(s, subtick, n->children + i, tid);
}

void scene_update(scene_t *s)
{
	for(int i = 0; i < s->num_anims; i++)
		animation_update(s->anims + i);
}

void scene_draw(const scene_t *s, float subtick, const uint32_t tid)
{
	animation_debug(s->anims);
	_scene_node_mesh_draw(s, subtick, &s->root_node, tid);
}
