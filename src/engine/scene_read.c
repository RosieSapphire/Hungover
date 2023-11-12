#include "engine/scene.h"

static void _mesh_read(smesh_t *m, FILE *file)
{
	fread(m->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	fread(&m->num_verts, sizeof(u16), 1, file);
	m->verts = malloc(sizeof(struct vertex) * m->num_verts);

	for (int i = 0; i < m->num_verts; i++)
	{
		fread(m->verts[i].pos, sizeof(f32), 3, file);
		fread(m->verts[i].uv, sizeof(f32), 2, file);
	}

	fread(&m->num_indis, sizeof(u16), 1, file);
	m->indis = malloc(sizeof(u16) * m->num_indis);

	for (int i = 0; i < m->num_indis; i++)
		fread(m->indis + i, sizeof(u16), 1, file);

	fread(&m->tex_index, sizeof(u16), 1, file);
}

static void _anim_read(animation_t *a, FILE *file)
{
	fread(a->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	fread(&a->mesh_index, sizeof(u16), 1, file);
	fread(&a->length, sizeof(u16), 1, file);
	fread(&a->num_pos, sizeof(u16), 1, file);
	fread(&a->num_rot, sizeof(u16), 1, file);
	fread(&a->num_sca, sizeof(u16), 1, file);
	a->frame = a->frame_last = 0;
	a->is_playing = a->loops = 1;
	a->pos = malloc(sizeof(vec3_key_t) * a->num_pos);

	for (u16 i = 0; i < a->num_pos; i++)
	{
		fread(&a->pos[i].frame, sizeof(u16), 1, file);
		fread(a->pos[i].vec, sizeof(f32), 3, file);
	}

	a->rot = malloc(sizeof(vec4_key_t) * a->num_rot);

	for (u16 i = 0; i < a->num_rot; i++)
	{
		fread(&a->rot[i].frame, sizeof(u16), 1, file);
		fread(a->rot[i].vec, sizeof(f32), 4, file);
	}

	a->sca = malloc(sizeof(vec3_key_t) * a->num_sca);

	for (u16 i = 0; i < a->num_sca; i++)
	{
		fread(&a->sca[i].frame, sizeof(u16), 1, file);
		fread(a->sca[i].vec, sizeof(f32), 3, file);
	}
}

static void _node_read(node_t *n, FILE *file, int depth)
{
	fread(n->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	fread(&n->mesh_index, sizeof(u16), 1, file);
	fread(n->mat, sizeof(f32), 4 * 4, file);
	fread(&n->num_children, sizeof(u16), 1, file);
	n->is_active = true;
	for (int i = 0; i < depth; i++)
		debugf("\t");
	debugf("name=%s, mesh_index=%d num_children=%d\n",
			n->name, n->mesh_index, n->num_children);

	n->children = malloc(sizeof(node_t) * n->num_children);
	for (int i = 0; i < n->num_children; i++)
		_node_read(n->children + i, file, depth + 1);
}

scene_t *scene_load(const char *path)
{
	FILE *file = fopen(path, "rb");

	if (!file)
	{
		debugf("FAILED TO LOAD SCENE FROM '%s'\n", path);
		exit(1);
	}

	scene_t *scene = malloc(sizeof(scene_t));

	fread(&scene->num_meshes, sizeof(u16), 1, file);
	scene->meshes = malloc(sizeof(smesh_t) * scene->num_meshes);
	for (int i = 0; i < scene->num_meshes; i++)
		_mesh_read(scene->meshes + i, file);

	fread(&scene->num_anims, sizeof(u16), 1, file);
	scene->anims = malloc(sizeof(animation_t) * scene->num_anims);
	for (int i = 0; i < scene->num_anims; i++)
		_anim_read(scene->anims + i, file);

	_node_read(&scene->root_node, file, 0);

	fread(&scene->num_tex_indis, sizeof(u16), 1, file);
	scene->tex_indis = malloc(sizeof(u32) * scene->num_tex_indis);
	for (u16 i = 0; i < scene->num_tex_indis; i++)
	{
		char buf[CONF_TEX_PATH_MAX_LEN];

		fread(buf, sizeof(char), CONF_TEX_PATH_MAX_LEN, file);
		scene->tex_indis[i] = texture_create_file(buf);
	}

	fclose(file);

	return (scene);
}

void scene_unload(scene_t *s)
{
	for (int i = 0; i < s->num_meshes; i++)
		smesh_destroy(s->meshes + i);

	for (int i = 0; i < s->num_anims; i++)
		animation_destroy(s->anims + i);

	node_destroy(&s->root_node);

	free(s);
}

