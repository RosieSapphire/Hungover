#ifndef _SCENE_H_
#define _SCENE_H_

#include "object.h"
#include "texture.h"

#define SCENE_NUM_MODELS_CACHED_MAX 512
#define SCENE_NUM_OBJECTS_MAX 1024
#define SCENE_NUM_TEXTURES_MAX 1024

typedef struct {
	vec4 bg_color;
	vec4 light_color;
	uint16_t num_models_cached, num_objects, num_textures;
	model_t models_cached[SCENE_NUM_MODELS_CACHED_MAX];
	object_t objects[SCENE_NUM_OBJECTS_MAX];
	texture_t textures[SCENE_NUM_TEXTURES_MAX];
} scene_t;

scene_t scene_create_default(void);
void scene_import_model_to_object(scene_t *s, const char *mdl_path,
				  int *obj_selected);
void scene_object_remove_from_list(scene_t *s, const unsigned int index);
scene_t scene_import(const char *path);
void scene_export(const scene_t *s, const char *path);
void scene_destroy(scene_t *s);

#endif /* _SCENE_H_ */
