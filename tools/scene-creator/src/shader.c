#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <GL/glew.h>

#include "error.h"
#include "shader.h"

static uint32_t _shader_compile(const char *path, const int type)
{
	const char *type_string = (type == GL_VERTEX_SHADER) ? "vertex" :
							       "fragment";
	FILE *file = fopen(path, "r");
	if (!file)
		error_log("Failed to load %s shader code from '%s'.",
			  type_string, path);
	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	rewind(file);
	char *source = calloc(file_size + 1, 1);
	fread(source, 1, file_size, file);
	uint32_t shader = glCreateShader(type);
	glShaderSource(shader, 1, (const char *const *)&source, NULL);
	free(source);
	glCompileShader(shader);
	int32_t status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char log[512];
		glGetShaderInfoLog(shader, 512, NULL, log);
		error_log("Failed to compile %s shader: %s", type_string, log);
	}
	return shader;
}

int32_t shader_uniform_get_and_verify(const uint32_t prog, const char *loc)
{
	int32_t uni = glGetUniformLocation(prog, loc);

	if (uni == -1) {
		error_log("Failed to find uniform '%s' in shader program.",
			  loc);
	}

	return uni;
}

shader_t shader_create_from_file(const char *vs_path, const char *fs_path,
				 const uint8_t uniform_flags)
{
	shader_t shader;
	memset(&shader, 0, sizeof shader);
	uint32_t vs = _shader_compile(vs_path, GL_VERTEX_SHADER);
	glAttachShader(shader.program, vs);
	uint32_t fs = _shader_compile(fs_path, GL_FRAGMENT_SHADER);
	shader.program = glCreateProgram();
	glAttachShader(shader.program, vs);
	glAttachShader(shader.program, fs);
	glLinkProgram(shader.program);
	int32_t status;
	glGetProgramiv(shader.program, GL_LINK_STATUS, &status);
	if (!status) {
		char log[512];
		glGetProgramInfoLog(shader.program, 512, NULL, log);
		error_log("Failed to link shader program: %s", log);
	}
	glDeleteShader(vs);
	glDeleteShader(fs);

	shader.uniform_flags = uniform_flags;
	if (uniform_flags & SHADER_INCLUDE_UNIFORM_TEXTURE) {
		shader.texture_uni = shader_uniform_get_and_verify(
			shader.program, "u_texture");
		shader.is_using_texture = shader_uniform_get_and_verify(
			shader.program, "u_is_using_texture");
	}
	if (uniform_flags & SHADER_INCLUDE_UNIFORM_PROJECTION) {
		shader.proj_matrix_uni = shader_uniform_get_and_verify(
			shader.program, "u_proj_matrix");
	}
	if (uniform_flags & SHADER_INCLUDE_UNIFORM_VIEW) {
		shader.view_matrix_uni = shader_uniform_get_and_verify(
			shader.program, "u_view_matrix");
	}
	if (uniform_flags & SHADER_INCLUDE_UNIFORM_MODEL) {
		shader.model_matrix_uni = shader_uniform_get_and_verify(
			shader.program, "u_model_matrix");
	}

	return shader;
}

void shader_destroy(shader_t *s)
{
	glDeleteProgram(s->program);
	s->uniform_flags = 0;
	s->texture_uni = s->is_using_texture = s->proj_matrix_uni =
		s->view_matrix_uni = s->model_matrix_uni = -1;
}
