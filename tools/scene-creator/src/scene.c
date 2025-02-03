#include <string.h>
#include <errno.h>

#include "util.h"
#include "scene.h"

scene_t scene_create_default(void)
{
	scene_t s;

	glm_vec4_copy((vec4){ .1f, .2f, .3f, 1.f }, s.bg_color);
	glm_vec4_copy((vec4){ 1.f, 1.f, 1.f, 1.f }, s.light_color);
	s.num_models_cached = s.num_objects = s.num_textures = 0;
	memset(s.models_cached, 0,
	       SCENE_NUM_MODELS_CACHED_MAX * sizeof *s.models_cached);
	memset(s.objects, 0, SCENE_NUM_OBJECTS_MAX * sizeof *s.objects);
	memset(s.textures, 0, SCENE_NUM_TEXTURES_MAX * sizeof *s.textures);

	return s;
}

void scene_import_model_to_object(scene_t *s, const char *mdl_path,
				  int *obj_selected)
{
	s->models_cached[s->num_models_cached++] = model_create_from_file(
		mdl_path, MODEL_FLAG_IS_ACTIVE, &s->num_textures, s->textures);
	int new_mdl_is_identical = 0;
	int index_of_identical_mdl = -1;
	for (uint16_t i = 0; i < s->num_models_cached - 1; i++) {
		model_t *mdl_i = s->models_cached + i;
		model_t *mdl_cmp = s->models_cached + s->num_models_cached - 1;
		if (models_are_identical(mdl_i, mdl_cmp)) {
			model_destroy(mdl_cmp);
			s->num_models_cached--;
			new_mdl_is_identical = 1;
			index_of_identical_mdl = i;
			break;
		}
	}

	*obj_selected = s->num_objects++;
	s->objects[*obj_selected] = object_create_from_model(
		new_mdl_is_identical ?
			(s->models_cached + index_of_identical_mdl) :
			(s->models_cached + s->num_models_cached - 1),
		OBJECT_FLAG_IS_ACTIVE);
}

static void _scene_remove_model_from_list(scene_t *s, const unsigned int index)
{
	model_t *mdl = s->models_cached + index;

	/* if this is the last model in the list */
	model_destroy(mdl);
	if (index + 1 >= s->num_models_cached) {
		s->num_models_cached--;
		return;
	}

	for (unsigned int i = index + 1; i < s->num_models_cached; i++) {
		model_t *to = s->models_cached + i - 1;
		model_t *from = s->models_cached + i;

		model_move(to, from);

		/* Once the model is removed from the slot in memory,
		 * the pointers for the objects using the models need
		 * to be shifted, otherwise the object is using a
		 * stale pointer. This code does exactly that. */
		for (unsigned int j = 0; j < s->num_objects; j++) {
			object_t *o = s->objects + j;
			if (o->mdl == from) {
				o->mdl = to;
			}
		}
	}
	model_destroy(s->models_cached + --s->num_models_cached);
}

static void _scene_remove_texture_from_list(scene_t *s,
					    const unsigned int index)
{
	/* if this is the last texture in the list */
	texture_destroy(s->textures, index);
	if (index + 1 >= s->num_textures) {
		--s->num_textures;
		return;
	}

	for (unsigned int i = index + 1; i < s->num_textures; i++) {
		const unsigned int to_ind = i - 1;
		const unsigned int from_ind = i;
		texture_t *to = s->textures + to_ind;
		texture_t *from = s->textures + from_ind;

		texture_move(to, from);

		for (unsigned int j = 0; j < s->num_models_cached; j++) {
			model_t *mdl = s->models_cached + j;
			for (unsigned int k = 0; k < mdl->num_meshes; k++) {
				mesh_t *msh = mdl->meshes + k;
				if (msh->tex_ind == from_ind) {
					msh->tex_ind = to_ind;
				}
			}
		}
	}
	texture_destroy(s->textures, --s->num_textures);
}

static void _scene_texture_list_remove_unused(scene_t *s)
{
	for (unsigned int i = 0; i < s->num_textures; i++) {
		int texture_is_in_use = 0;
		for (unsigned int j = 0; j < s->num_models_cached; j++) {
			model_t *mdl = s->models_cached + j;
			for (unsigned int k = 0; k < mdl->num_meshes; k++) {
				mesh_t *msh = mdl->meshes + k;
				if (msh->tex_ind == i) {
					texture_is_in_use = 1;
					break;
				}
			}
		}
		if (!texture_is_in_use) {
			_scene_remove_texture_from_list(s, i);
		}
	}
}

static void _scene_model_list_remove_unused(scene_t *s)
{
	for (unsigned int i = 0; i < s->num_models_cached; i++) {
		int model_is_in_use = 0;
		model_t *m = s->models_cached + i;
		for (unsigned int j = 0; j < s->num_objects; j++) {
			object_t *o = s->objects + j;
			if (o->mdl == m) {
				model_is_in_use = 1;
				break;
			}
		}
		if (!model_is_in_use) {
			_scene_remove_model_from_list(s, i);
			_scene_texture_list_remove_unused(s);
		}
	}
}

void scene_object_remove_from_list(scene_t *s, const unsigned int index)
{
	object_t *obj = s->objects + index;

	/* if this is the last object in the list */
	object_destroy(obj);
	if (index + 1 >= s->num_objects) {
		s->num_objects--;
		_scene_model_list_remove_unused(s);
		return;
	}

	for (unsigned int i = index + 1; i < s->num_objects; i++) {
		object_t *to = s->objects + i - 1;
		object_t *from = s->objects + i;

		object_duplicate(to, from);
	}
	object_destroy(s->objects + --s->num_objects);
	_scene_model_list_remove_unused(s);
}

/* Scene file format:
 *
 * bg_col (4 floats)
 * light_col (4 floats)
 * num_models_cached (uint16_t)
 * models_cached:
 * 	- pathlen (uint16_t)
 * 	- path (maximum of 512 chars)
 * 	- position (3 floats)
 * 	- flags (uint8_t)
 * num_objects (uint16_t)
 * objects:
 * 	- model index (uint16_t)
 * 	- position (3 floats)
 * 	- flags (uint8_t)
 */

static void _scene_import_model(model_t *m, FILE *file)
{
	uint16_t pathlen = 0;

	fread_u16_flip(&pathlen, file);
	memset(m->path, 0, MODEL_PATH_MAX_LEN);
	fread(m->path, 1, pathlen, file);
	fread_fvec_flip(m->position, 3, file);
	fread(&m->flags, 1, 1, file);
}

static void _scene_import_object(object_t *o, model_t *mdl_array, FILE *file)
{
	uint16_t mdl_ind = 0;

	fread_u16_flip(&mdl_ind, file);
	fread_fvec_flip(o->position, 3, file);
	fread(&o->flags, 1, 1, file);

	o->mdl = mdl_array + mdl_ind;
}

scene_t scene_import(const char *path)
{
	scene_t s;
	FILE *file = fopen(path, "rb");

	if (!file) {
		fprintf(stderr, "Failed to load '%s' to scene. Using default\n",
			path);
		return scene_create_default();
	}

	fread_fvec_flip(s.bg_color, 4, file);
	fread_fvec_flip(s.light_color, 4, file);

	fread_u16_flip(&s.num_models_cached, file);
	memset(s.models_cached, 0,
	       sizeof *s.models_cached * SCENE_NUM_MODELS_CACHED_MAX);
	for (uint16_t i = 0; i < s.num_models_cached; i++) {
		_scene_import_model(s.models_cached + i, file);
	}

	fread_u16_flip(&s.num_objects, file);
	memset(s.objects, 0, sizeof *s.objects * SCENE_NUM_OBJECTS_MAX);
	for (uint16_t i = 0; i < s.num_objects; i++) {
		_scene_import_object(s.objects + i, s.models_cached, file);
	}

	return s;
}

static void _scene_export_model(const model_t *m, FILE *file)
{
	uint16_t pathlen = strlen(m->path);

	fwrite_u16_flip(&pathlen, file);
	fwrite(m->path, pathlen, 1, file);
	fwrite_fvec_flip(m->position, 3, file);
	fwrite(&m->flags, 1, 1, file);
}

static void _scene_export_object(const object_t *o, const model_t *mdl_array,
				 FILE *file)
{
	uint16_t mdl_ind = o->mdl - mdl_array;

	fwrite_u16_flip(&mdl_ind, file);
	fwrite_fvec_flip(o->position, 3, file);
	fwrite(&o->flags, 1, 1, file);
}

void scene_export(const scene_t *s, const char *path)
{
	FILE *file = fopen(path, "wb");

	if (!file) {
		fprintf(stderr, "Failed to open file '%s' (%s)\n", path,
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	fwrite_fvec_flip(s->bg_color, 4, file);
	fwrite_fvec_flip(s->light_color, 4, file);

	fwrite_u16_flip(&s->num_models_cached, file);
	for (uint16_t i = 0; i < s->num_models_cached; i++) {
		_scene_export_model(s->models_cached + i, file);
	}

	fwrite_u16_flip(&s->num_objects, file);
	for (uint16_t i = 0; i < s->num_models_cached; i++) {
		_scene_export_object(s->objects + i, s->models_cached, file);
	}

	// I might not need to write this at all.
	//
	// fwrite_u16_flip(&s->num_textures, file);
	// texture_t textures[SCENE_NUM_TEXTURES_MAX];

	fclose(file);
}

void scene_destroy(scene_t *s)
{
	glm_vec4_zero(s->bg_color);
	glm_vec4_zero(s->light_color);
	for (uint32_t i = 0; i < s->num_models_cached; i++) {
		model_destroy(s->models_cached + i);
	}
	memset(s->models_cached, 0,
	       sizeof *s->models_cached * SCENE_NUM_MODELS_CACHED_MAX);
	for (uint32_t i = 0; i < s->num_objects; i++) {
		object_destroy(s->objects + i);
	}
	memset(s->objects, 0, sizeof *s->objects * SCENE_NUM_OBJECTS_MAX);
	for (uint32_t i = 0; i < s->num_objects; i++) {
		texture_destroy(s->textures, i);
	}
	memset(s->textures, 0, sizeof *s->textures * SCENE_NUM_TEXTURES_MAX);
	s->num_models_cached = s->num_objects = s->num_textures = 0;
}
