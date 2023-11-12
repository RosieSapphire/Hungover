#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "mesh.h"

/**
 * mesh_index_from_name - Gets Mesh Index from Name
 * @name: Name to Find
 * @meshes: Meshes Array
 * @num_meshes: Number of Meshes
 *
 * Return: Corresponding Mesh Index, or 0xFFFF for failure
 */
u16 mesh_index_from_name(const char *name, const struct mesh *meshes,
			 const u16 num_meshes)
{
	for (u16 i = 0; i < num_meshes; i++)
		if (strcmp(name, meshes[i].name) == 0)
			return (i);

	return (0xFFFF);
}

/**
 * _mesh_debug - Debugs information about a Mesh
 * @m: Mesh in Question
 */
static void _mesh_debug(const struct mesh *m)
{
	printf("\tname=%s, num_verts=%d, num_indis=%d, tex_index=%d\n",
			m->name, m->num_verts, m->num_indis, m->tex_index);

	for (u16 i = 0; i < m->num_verts; i++)
	{
		struct vertex *v = m->verts + i;

		printf("\t\t%d: pos=(%f, %f, %f), uv=(%f, %f)\n", i,
				v->pos[0], v->pos[1], v->pos[2],
				v->uv[0], v->uv[1]);
	}

	printf("\n");

	for (u16 i = 0; i < m->num_indis / 3; i++)
		printf("\t\t%d\t%d\t%d\n", m->indis[i * 3 + 0],
			m->indis[i * 3 + 1], m->indis[i * 3 + 2]);

	printf("\n");
}

/**
 * mesh_read - Reads a Mesh from a File
 * @m: Mesh to read To
 * @file: File to read From
 */
void mesh_read(struct mesh *m, FILE *file)
{
	fread(m->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	fread(&m->num_verts, sizeof(u16), 1, file);
	m->num_verts = u16_endian_flip(m->num_verts);
	m->verts = malloc(sizeof(struct vertex) * m->num_verts);
	for (int i = 0; i < m->num_verts; i++)
	{
		fread(m->verts[i].pos, sizeof(f32), 3, file);
		for (int j = 0; j < 3; j++)
			m->verts[i].pos[j] =
				f32_endian_flip(m->verts[i].pos[j]);

		fread(m->verts[i].uv, sizeof(f32), 2, file);
		for (int j = 0; j < 2; j++)
			m->verts[i].uv[j] = f32_endian_flip(m->verts[i].uv[j]);
	}

	fread(&m->num_indis, sizeof(u16), 1, file);
	m->num_indis = u16_endian_flip(m->num_indis);
	m->indis = malloc(sizeof(u16) * m->num_indis);
	for (int i = 0; i < m->num_indis; i++)
	{
		fread(m->indis + i, sizeof(u16), 1, file);
		m->indis[i] = u16_endian_flip(m->indis[i]);
	}

	fread(&m->tex_index, sizeof(u16), 1, file);
	m->tex_index = u16_endian_flip(m->tex_index);

	_mesh_debug(m);
}

/**
 * mesh_write - Writes a Mesh to a File
 * @m: Mesh to write from
 * @file: File to write to
 */
void mesh_write(const struct mesh *m, FILE *file)
{
	u16 num_verts_flip = u16_endian_flip(m->num_verts);

	fwrite(m->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	fwrite(&num_verts_flip, sizeof(u16), 1, file);
	for (u16 i = 0; i < m->num_verts; i++)
	{
		f32 pflip[3] = {
			f32_endian_flip(m->verts[i].pos[0]),
			f32_endian_flip(m->verts[i].pos[1]),
			f32_endian_flip(m->verts[i].pos[2])
		};
		f32 uflip[2] = {
			f32_endian_flip(m->verts[i].uv[0]),
			f32_endian_flip(m->verts[i].uv[1]),
		};

		fwrite(pflip, sizeof(f32), 3, file);
		fwrite(uflip, sizeof(f32), 2, file);
	}

	u16 num_indis_flip = u16_endian_flip(m->num_indis);

	fwrite(&num_indis_flip, sizeof(u16), 1, file);
	for (u16 i = 0; i < m->num_indis; i++)
	{
		u16 iflip = u16_endian_flip(m->indis[i]);

		fwrite(&iflip, sizeof(u16), 1, file);
	}

	u16 tex_index_flip = u16_endian_flip(m->tex_index);

	fwrite(&tex_index_flip, sizeof(u16), 1, file);
}
