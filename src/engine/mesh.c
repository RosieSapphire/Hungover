#include <malloc.h>
#include <string.h>

#include <GL/gl.h>

#include "engine/scene.h"
#include "engine/mesh.h"

/**
 * mesh_create_data - Creates a Mesh from Data
 * @name: Mesh Name
 * @num_verts: Number of Vertices
 * @num_indis: Number of Indices
 * @verts: Vertices Array
 * @indis: Indices Array
 *
 * Return: Mesh Created from Data
 */
struct mesh *mesh_create_data(const char *name, u16 num_verts, u16 num_indis,
			      const struct vertex *verts, const u16 *indis)
{
	const size_t verts_size = sizeof(struct vertex) * num_verts;
	const size_t indis_size = sizeof(u16) * num_indis;
	struct mesh *m = malloc(sizeof(struct mesh));

	strcpy(m->name, name);
	m->verts = malloc(verts_size);
	m->indis = malloc(indis_size);
	m->num_verts = num_verts;
	m->num_indis = num_indis;
	memcpy(m->verts, verts, verts_size);
	memcpy(m->indis, indis, indis_size);

	return (m);
}

/**
 * mesh_copy - Copies a Mesh to Another
 * @src: Source Mesh
 * @dst: Destination Mesh
 */
void mesh_copy(const struct mesh *src, struct mesh *dst)
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

/**
 * mesh_destroy - Destroys/Unloads a Mesh
 * @m: Mesh to Unload
 */
void mesh_destroy(struct mesh *m)
{
	free(m->verts);
	free(m->indis);
	m->num_verts = m->num_indis = 0;
}

/**
 * mesh_draw - Draws a Mesh in a Scene
 * @sc: Scene to Connect To
 * @m: Mesh to Draw
 *
 * Description: Draws a Mesh using a Scene to find Texture Index
 */
void mesh_draw(const void *sc, const struct mesh *m)
{
	struct scene *s = (struct scene *)sc;

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

/**
 * mesh_draw_tex - Draws a Mesh with a custom Texture
 * @m: Mesh to Draw
 * @tex: OpenGL Texture ID
 */
void mesh_draw_tex(const struct mesh *m, const u32 tex)
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
