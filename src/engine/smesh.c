#include <malloc.h>
#include <string.h>

#include <GL/gl.h>

#include "engine/scene.h"
#include "engine/smesh.h"

/**
 * smesh_create_data - Creates a Mesh from Data
 * @name: Mesh Name
 * @num_verts: Number of Vertices
 * @num_indis: Number of Indices
 * @verts: Vertices Array
 * @indis: Indices Array
 *
 * Return: Mesh Created from Data
 */
smesh_t *smesh_create_data(const char *name, u16 num_verts,
		u16 num_indis, const struct vertex *verts, const u16 *indis)
{
	const size_t verts_size = sizeof(struct vertex) * num_verts;
	const size_t indis_size = sizeof(u16) * num_indis;
	smesh_t *m = malloc(sizeof(smesh_t));

	strcpy(m->name, name);
	m->verts = malloc(verts_size);
	m->indis = malloc(indis_size);
	m->num_verts = num_verts;
	m->num_indis = num_indis;
	memcpy(m->verts, verts, verts_size);
	memcpy(m->indis, indis, indis_size);

	return (m);
}

void smesh_copy(const smesh_t *src, smesh_t *dst)
{
	const size_t verts_size = sizeof(struct vertex) * src->num_verts;
	const size_t indis_size = sizeof(u16) * src->num_indis;

	strncpy(dst->name, src->name, CONF_NAME_MAX_LEN);
	dst->num_verts = src->num_verts;
	dst->num_indis = src->num_indis;
	dst->verts = malloc(verts_size);
	dst->indis = malloc(indis_size);
	memcpy(dst->verts, src->verts, verts_size);
	memcpy(dst->indis, src->indis, indis_size);
}

void smesh_destroy(smesh_t *m)
{
	free(m->verts);
	free(m->indis);
	m->num_verts = m->num_indis = 0;
}

void smesh_draw(const void *sc, const smesh_t *m)
{
	scene_t *s = (scene_t *)sc;

	if (s)
	{
		const int tex_ind = s->tex_indis[m->tex_index];

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_objs_loaded[tex_ind].id);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 5 * sizeof(f32), m->verts->pos);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(f32), m->verts->uv);

	glDrawElements(GL_TRIANGLES, m->num_indis,
			GL_UNSIGNED_SHORT, m->indis);

	if (s)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void smesh_draw_tex(const smesh_t *m, const u32 tex)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 5 * sizeof(f32), m->verts->pos);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(f32), m->verts->uv);

	glDrawElements(GL_TRIANGLES, m->num_indis,
			GL_UNSIGNED_SHORT, m->indis);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}
