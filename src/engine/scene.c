#include <stdio.h>
#include <malloc.h>

#include "engine/scene.h"

enum scene_index scene_index = SCENE_TESTROOM;

static void _mesh_read(smesh_t *m, FILE *file)
{
	fread(m->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	fread(&m->num_verts, sizeof(uint16_t), 1, file);
	m->verts = malloc(sizeof(vertex_t) * m->num_verts);
	for(int i = 0; i < m->num_verts; i++) {
		fread(m->verts[i].pos, sizeof(float), 3, file);
		fread(m->verts[i].uv, sizeof(float), 2, file);
	}

	fread(&m->num_indis, sizeof(uint16_t), 1, file);
	m->indis = malloc(sizeof(uint16_t) * m->num_indis);
	for(int i = 0; i < m->num_indis; i++)
		fread(m->indis + i, sizeof(uint16_t), 1, file);

	// debugging
	debugf("\tname=%s, num_verts=%d, num_indis=%d\n",
			m->name, m->num_verts, m->num_indis);

	for(uint16_t i = 0; i < m->num_verts; i++) {
		vertex_t *v = m->verts + i;
		debugf("\t\t%d: pos=(%f, %f, %f), uv=(%f, %f)\n", i,
				v->pos[0], v->pos[1], v->pos[2],
				v->uv[0], v->uv[1]);
	}

	debugf("\n");

	for(uint16_t i = 0; i < m->num_indis / 3; i++)
		debugf("\t\t%d\t%d\t%d\n",
				m->indis[i * 3 + 0],
				m->indis[i * 3 + 1],
				m->indis[i * 3 + 2]);

	debugf("\n");
}

static void _anim_read(animation_t *a, FILE *file)
{
	fread(a->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	fread(&a->mesh_index, sizeof(uint16_t), 1, file);
	fread(&a->length, sizeof(uint16_t), 1, file);
	fread(&a->num_pos, sizeof(uint16_t), 1, file);
	fread(&a->num_rot, sizeof(uint16_t), 1, file);
	fread(&a->num_sca, sizeof(uint16_t), 1, file);

	a->pos = malloc(sizeof(vec3_key_t) * a->num_pos);
	for(uint16_t i = 0; i < a->num_pos; i++) {
		fread(&a->pos[i].frame, sizeof(uint16_t), 1, file);
		fread(a->pos[i].vec, sizeof(float), 3, file);
	}

	a->rot = malloc(sizeof(vec4_key_t) * a->num_rot);
	for(uint16_t i = 0; i < a->num_rot; i++) {
		fread(&a->rot[i].frame, sizeof(uint16_t), 1, file);
		fread(a->rot[i].vec, sizeof(float), 4, file);
	}

	a->sca = malloc(sizeof(vec3_key_t) * a->num_sca);
	for(uint16_t i = 0; i < a->num_sca; i++) {
		fread(&a->sca[i].frame, sizeof(uint16_t), 1, file);
		fread(a->sca[i].vec, sizeof(float), 3, file);
	}

	// debugging
	debugf("\tname=%s, mesh_index=%d, length=%d, "
			"npos=%d, nrot=%d, nsca=%d\n",
			a->name, a->mesh_index, a->length,
			a->num_pos, a->num_rot, a->num_sca);

	for(uint16_t i = 0; i < a->num_pos; i++)
		debugf("\t\tpos%d=(%f, %f, %f)\n", a->pos[i].frame,
				a->pos[i].vec[0], a->pos[i].vec[1],
				a->pos[i].vec[2]);

	debugf("\n");

	for(uint16_t i = 0; i < a->num_rot; i++)
		debugf("\t\trot%d=(%f, %f, %f, %f)\n", a->rot[i].frame,
				a->rot[i].vec[0], a->rot[i].vec[1],
				a->rot[i].vec[2], a->rot[i].vec[3]);

	debugf("\n");

	for(uint16_t i = 0; i < a->num_sca; i++)
		debugf("\t\tsca%d=(%f, %f, %f)\n", a->sca[i].frame,
				a->sca[i].vec[0], a->sca[i].vec[1],
				a->sca[i].vec[2]);

	debugf("\n");
}

static void _node_read(node_t *n, FILE *file, int depth)
{
	fread(n->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	fread(&n->mesh_index, sizeof(uint16_t), 1, file);
	fread(n->mat, sizeof(float), 4 * 4, file);
	fread(&n->num_children, sizeof(uint16_t), 1, file);
	n->is_active = true;
	for(int i = 0; i < depth; i++)
		debugf("\t");
	debugf("name=%s, mesh_index=%d num_children=%d\n",
			n->name, n->mesh_index, n->num_children);

	n->children = malloc(sizeof(node_t) * n->num_children);
	for(int i = 0; i < n->num_children; i++)
		_node_read(n->children + i, file, depth + 1);
}

scene_t *scene_load(const char *path)
{
	FILE *file = fopen(path, "rb");
	scene_t *scene = malloc(sizeof(scene_t));

	fread(&scene->num_meshes, sizeof(uint16_t), 1, file);
	scene->meshes = malloc(sizeof(smesh_t) * scene->num_meshes);
	debugf("num_meshes=%d\n", scene->num_meshes);
	for(int i = 0; i < scene->num_meshes; i++)
		_mesh_read(scene->meshes + i, file);

	fread(&scene->num_anims, sizeof(uint16_t), 1, file);
	scene->anims = malloc(sizeof(animation_t) * scene->num_anims);
	debugf("num_anims=%d\n", scene->num_anims);
	for(int i = 0; i < scene->num_anims; i++)
		_anim_read(scene->anims + i, file);

	_node_read(&scene->root_node, file, 0);

	fclose(file);

	return scene;
}

void scene_unload(scene_t *s)
{
	for(int i = 0; i < s->num_meshes; i++)
		smesh_destroy(s->meshes + i);

	/*
	for(int i = 0; i < s->num_meshes; i++)
		animation_destroy(s->anims + i);
		*/

	free(s);
}

/*
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

static void _scene_mesh_draw(const smesh_t *m, float subtick, const uint32_t tid)
{
	const int anim_index = _scene_anim_index_from_mesh_index

	if(anim_index != 0xFFFF) {
		// animation_setup_matrix(s->anims + anim_index, subtick);
		smesh_draw(s->meshes + (n->mesh_index), tid);
	} else {
		smesh_draw(s->meshes + (n->mesh_index), tid);
	}
}
*/

void scene_update(scene_t *s)
{
	for(int i = 0; i < s->num_anims; i++)
		animation_update(s->anims + i);
}

static void _scene_node_draw(const scene_t *s, const node_t *n,
		float subtick, const uint32_t tid)
{
	if(!n->is_active)
		return;

	if(n->mesh_index == 0xFFFF) {
		for(int i = 0; i < n->num_children; i++) {
			_scene_node_draw(s, n->children + i, subtick, tid);
		}
		return;
	}

	uint16_t anim_index = 0xFFFF;
	for(uint16_t j = 0; j < s->num_anims; j++) {
		if(s->anims[j].mesh_index == n->mesh_index) {
			anim_index = j;
			break;
		}
	}

	glPushMatrix();
	if(anim_index != 0xFFFF)
		animation_setup_matrix(s->anims + anim_index, subtick);
	else
		glMultMatrixf(n->mat);

	smesh_draw(s->meshes + n->mesh_index, tid);
	for(int i = 0; i < n->num_children; i++) {
		_scene_node_draw(s, n->children + i, subtick, tid);
	}
	glPopMatrix();
}

void scene_draw(const scene_t *s, float subtick, const uint32_t tid)
{
	_scene_node_draw(s, &s->root_node, subtick, tid);
}

node_t *scene_node_from_name(node_t *n, const char *name)
{
	if(strcmp(n->name, name) == 0)
		return n;

	node_t *f = NULL;
	for(int i = 0; i < n->num_children; i++) {
		f = scene_node_from_name(n->children + i, name);
		if(f)
			return f;
	}

	return NULL;
}
