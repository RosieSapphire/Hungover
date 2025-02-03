#ifndef _SHADER_H_
#define _SHADER_H_

#include <stdint.h>

enum {
	SHADER_INCLUDE_UNIFORM_TEXTURE = (1 << 0),
	SHADER_INCLUDE_UNIFORM_PROJECTION = (1 << 1),
	SHADER_INCLUDE_UNIFORM_VIEW = (1 << 2),
	SHADER_INCLUDE_UNIFORM_MODEL = (1 << 3),
};

typedef struct {
	uint32_t program;
	uint8_t uniform_flags;
	int32_t texture_uni, is_using_texture, proj_matrix_uni, view_matrix_uni,
		model_matrix_uni;
} shader_t;

shader_t shader_create_from_file(const char *vs_path, const char *fs_path,
				 const uint8_t uniform_flags);
int32_t shader_uniform_get_and_verify(const uint32_t prog, const char *loc);
void shader_destroy(shader_t *s);

#endif /* _SHADER_H_ */
