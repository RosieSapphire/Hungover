#include <string.h>
#include <GL/glew.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"

static const uint32_t aabb_indis[6 * 6] = {
	0,  1,	2,  2,	1,  3,	4,  5,	6,  6,	5,  7,	8,  9,	10, 10, 9,  11,
	12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23,
};

void mesh_setup_buffers(mesh_t *m)
{
	glGenVertexArrays(1, &m->vao);
	glBindVertexArray(m->vao);

	glGenBuffers(1, &m->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			      (void *)offsetof(vertex_t, position));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			      (void *)offsetof(vertex_t, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			      (void *)offsetof(vertex_t, texcoord));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			      (void *)offsetof(vertex_t, color));
	glBufferData(GL_ARRAY_BUFFER, m->num_vertices * sizeof *m->vertices,
		     m->vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		     m->num_indices * sizeof *m->indices, m->indices,
		     GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	/* FIXME: Move this into the shader :3 */
	m->flags |= MESH_FLAG_IS_ACTIVE;
	m->tex_ind = (uint32_t)-1;
}

void mesh_generate_aabb(mesh_t *m)
{
	aabb_t *bb = &m->aabb;

	glGenVertexArrays(1, &bb->vao);
	glBindVertexArray(bb->vao);

	glGenBuffers(1, &bb->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, bb->vbo);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			      (void *)offsetof(vertex_t, position));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			      (void *)offsetof(vertex_t, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			      (void *)offsetof(vertex_t, texcoord));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			      (void *)offsetof(vertex_t, color));

	glm_vec3_zero(bb->min);
	glm_vec3_zero(bb->max);

	for (unsigned int i = 0; i < m->num_vertices; i++) {
		float *pos = m->vertices[i].position;

		for (int j = 0; j < 3; j++) {
			if (pos[j] < bb->min[j]) {
				bb->min[j] = pos[j];
			}

			if (pos[j] > bb->max[j]) {
				bb->max[j] = pos[j];
			}
		}
	}

	const vertex_t aabb_vertices[24] = {
		{ { m->aabb.min[0], m->aabb.min[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.min[0], m->aabb.max[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.min[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.max[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.min[0], m->aabb.min[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.min[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.min[0], m->aabb.max[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.max[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.min[0], m->aabb.min[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.min[0], m->aabb.min[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.min[0], m->aabb.max[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.min[0], m->aabb.max[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.min[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.max[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.min[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.max[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.min[0], m->aabb.max[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.min[0], m->aabb.max[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.max[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.max[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.min[0], m->aabb.min[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.min[1], m->aabb.min[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.min[0], m->aabb.min[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
		{ { m->aabb.max[0], m->aabb.min[1], m->aabb.max[2] },
		  { 0, 0, 0 },
		  { 0, 0 },
		  { 1, 1, 1, 1 } },
	};

	glBufferData(GL_ARRAY_BUFFER, AABB_NUM_VERTS * sizeof *aabb_vertices,
		     aabb_vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m->aabb.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->aabb.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(aabb_indis), aabb_indis,
		     GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

int meshes_are_identical(const mesh_t *a, const mesh_t *b)
{
	if (a->num_vertices != b->num_vertices) {
		return 0;
	}

	if (a->num_indices != b->num_indices) {
		return 0;
	}

	for (unsigned int i = 0; i < a->num_vertices; i++) {
		vertex_t *va = a->vertices + i;
		vertex_t *vb = b->vertices + i;
		for (int j = 0; j < 4; j++) {
			if (va->color[j] != vb->color[j]) {
				return 0;
			}
			if (j >= 3) {
				continue;
			}
			if (va->position[j] != vb->position[j]) {
				return 0;
			}
			if (va->normal[j] != vb->normal[j]) {
				return 0;
			}
			if (j >= 2) {
				continue;
			}
			if (va->texcoord[j] != vb->texcoord[j]) {
				return 0;
			}
		}
	}

	for (unsigned int i = 0; i < a->num_indices; i++) {
		if (a->indices[i] != b->indices[i]) {
			return 0;
		}
	}

	return 1;
}

void mesh_move(mesh_t *to, mesh_t *from)
{
	/* move from -> to */
	to->num_vertices = from->num_vertices;
	to->vertices = calloc(to->num_vertices, sizeof *to->vertices);
	memcpy(to->vertices, from->vertices,
	       to->num_vertices * sizeof *to->vertices);

	to->num_indices = from->num_indices;
	to->indices = calloc(to->num_indices, sizeof *to->indices);
	memcpy(to->indices, from->indices,
	       to->num_indices * sizeof *to->indices);
	mesh_setup_buffers(to);
	mesh_generate_aabb(to);
	glm_vec3_copy(from->position, to->position);
	to->flags = from->flags;
	strncpy(to->name, from->name, MESH_NAME_MAX_LEN);
	to->tex_ind = from->tex_ind;

	/* clear out from */
	from->num_vertices = from->num_indices = from->flags = 0;
	free(from->vertices);
	free(from->indices);
	glDeleteVertexArrays(1, &from->vao);
	glDeleteVertexArrays(1, &from->aabb.vao);
	glDeleteBuffers(1, &from->vbo);
	glDeleteBuffers(1, &from->aabb.vbo);
	glDeleteBuffers(1, &from->ebo);
	glDeleteBuffers(1, &from->aabb.ebo);
	glm_vec3_zero(from->position);
	glm_mat4_zero(from->matrix);
	memset(from->name, 0, MESH_NAME_MAX_LEN);
	from->tex_ind = -1;
}

void mesh_render(mesh_t *m, const shader_t *s, const float *proj_matrix,
		 const float *view_matrix, vec3 offset, texture_t *textures)
{
	if (!(m->flags & MESH_FLAG_IS_ACTIVE)) {
		return;
	}

	glm_mat4_identity(m->matrix);
	glm_translate(m->matrix, m->position);
	glm_translate(m->matrix, offset);

	glBindVertexArray(m->vao);

	const int is_using_tex = m->tex_ind != (uint32_t)-1;
	glUseProgram(s->program);
	glUniform1i(s->is_using_texture, is_using_tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,
		      is_using_tex ? textures[m->tex_ind].nkimg.handle.id : 0);

	glUniformMatrix4fv(s->proj_matrix_uni, 1, GL_FALSE,
			   (float *)proj_matrix);
	glUniformMatrix4fv(s->view_matrix_uni, 1, GL_FALSE,
			   (float *)view_matrix);
	glUniformMatrix4fv(s->model_matrix_uni, 1, GL_FALSE,
			   (float *)m->matrix);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_INT,
		       m->indices);
	glBindVertexArray(0);

	if (!(m->flags & MESH_FLAG_SHOW_AABB)) {
		return;
	}

	glBindVertexArray(m->aabb.vao);
	glDrawElements(GL_LINES, sizeof aabb_indis / sizeof *aabb_indis,
		       GL_UNSIGNED_INT, aabb_indis);
	glBindVertexArray(0);
}

void aabb_destroy(aabb_t *bb)
{
	glDeleteVertexArrays(1, &bb->vao);
	glDeleteBuffers(1, &bb->vbo);
	glDeleteBuffers(1, &bb->ebo);
	glm_vec3_zero(bb->min);
	glm_vec3_zero(bb->max);
}

void mesh_destroy(mesh_t *m)
{
	glDeleteVertexArrays(1, &m->vao);
	glDeleteBuffers(1, &m->vbo);
	glDeleteBuffers(1, &m->ebo);

	for (uint32_t i = 0; i < m->num_vertices; i++) {
		vertex_t *v = m->vertices + i;
		glm_vec3_zero(v->position);
		glm_vec3_zero(v->normal);
		glm_vec2_zero(v->texcoord);
		glm_vec4_zero(v->color);
	}
	m->num_vertices = 0;
	free(m->vertices);
	m->vertices = NULL;

	for (uint32_t i = 0; i < m->num_indices; i++) {
		m->indices[i] = 0;
	}
	m->num_indices = 0;
	free(m->indices);
	m->indices = NULL;

	glm_mat4_zero(m->matrix);
	glm_vec3_zero(m->position);
	m->flags = 0;
	aabb_destroy(&m->aabb);
	m->tex_ind = -1;
	memset(m->name, 0, MESH_NAME_MAX_LEN);
}
