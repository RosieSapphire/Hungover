#ifndef _MESH_H_
#define _MESH_H_

#include <cglm/cglm.h>

#include "shader.h"
#include "texture.h"

#define MESH_NAME_MAX_LEN 64

#define AABB_NUM_VERTS 24

enum {
	MESH_FLAG_IS_ACTIVE = (1 << 0),
	MESH_FLAG_SHOW_AABB = (1 << 1),
};

typedef struct {
	vec3 position, normal;
	vec2 texcoord;
	vec4 color;
} vertex_t;

typedef struct {
	uint32_t vao, vbo, ebo;
	vec3 min, max;
} aabb_t;

typedef struct {
	uint32_t vao, vbo, ebo;
	uint32_t num_vertices, num_indices;
	vertex_t *vertices;
	uint32_t *indices;
	mat4 matrix;
	vec3 position;
	uint8_t flags;
	aabb_t aabb;
	uint32_t tex_ind;
	char name[MESH_NAME_MAX_LEN];
} mesh_t;

void mesh_setup_buffers(mesh_t *m);
void mesh_generate_aabb(mesh_t *m);
int meshes_are_identical(const mesh_t *a, const mesh_t *b);
void mesh_move(mesh_t *to, mesh_t *from);
void mesh_render(mesh_t *m, const shader_t *s, const float *proj_matrix,
		 const float *view_matrix, vec3 offset, texture_t *textures);
void aabb_destroy(aabb_t *bb);
void mesh_destroy(mesh_t *m);

#endif /* _MESH_H_ */
