#include <malloc.h>
#include <string.h>

#include <GL/gl.h>

#include "engine/smesh.h"

smesh_t *smesh_create_data(uint16_t num_verts, uint16_t num_indis,
		const vertex_t *verts, const uint16_t *indis)
{
	smesh_t *m = malloc(sizeof(smesh_t));
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

void smesh_destroy(smesh_t *m)
{
	free(m->verts);
	free(m->indis);
}

void smesh_draw(const smesh_t *m, const uint32_t tid)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tid);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 5 * sizeof(float), m->verts->pos);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(float), m->verts->uv);

	glDrawElements(GL_TRIANGLES, m->num_indis,
			GL_UNSIGNED_SHORT, m->indis);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}
