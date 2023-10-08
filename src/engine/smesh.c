#include <malloc.h>
#include <string.h>

#include <GL/gl.h>

#include "engine/scene.h"
#include "engine/smesh.h"

smesh_t *smesh_create_data(const char *name, uint16_t num_verts,
		uint16_t num_indis, const vertex_t *verts,
		const uint16_t *indis)
{
	smesh_t *m = malloc(sizeof(smesh_t));
	strcpy(m->name, name);
	const size_t verts_size = sizeof(vertex_t) * num_verts;
	const size_t indis_size = sizeof(uint16_t) * num_indis;
	m->verts = malloc(verts_size);
	m->indis = malloc(indis_size);
	m->num_verts = num_verts;
	m->num_indis = num_indis;
	memcpy(m->verts, verts, verts_size);
	memcpy(m->indis, indis, indis_size);
	return m;
}

void smesh_copy(const smesh_t *src, smesh_t *dst)
{
	strncpy(dst->name, src->name, CONF_NAME_MAX_LEN);
	dst->num_verts = src->num_verts;
	dst->num_indis = src->num_indis;

	const size_t verts_size = sizeof(vertex_t) * src->num_verts;
	const size_t indis_size = sizeof(uint16_t) * src->num_indis;
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

	if(s) {
		const int tex_ind = s->tex_indis[m->tex_index];
		// debugf("%s:\t%d\n", m->name, tex_ind);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_objs_loaded[tex_ind].id);
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 5 * sizeof(float), m->verts->pos);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(float), m->verts->uv);

	glDrawElements(GL_TRIANGLES, m->num_indis,
			GL_UNSIGNED_SHORT, m->indis);

	if(s) {
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}
