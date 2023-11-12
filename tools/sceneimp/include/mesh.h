#ifndef _MESH_H_
#define _MESH_H_

#include <stdio.h>
#include "../../../include/engine/types.h"
#include "../../../include/engine/config.h"

#include "util.h"
#include "vertex.h"

/**
 * struct mesh - Mesh Struct
 * @name: Mesh Name
 * @num_verts: Number of Vertices
 * @num_indis: Number of Indices
 * @verts: Vertices Array
 * @indis: Indices Array
 * @tex_index: Texture Index
 */
struct mesh
{
	char name[CONF_NAME_MAX_LEN];
	u16 num_verts, num_indis;
	struct vertex *verts;
	u16 *indis;
	u16 tex_index;
};

u16 mesh_index_from_name(const char *name, const struct mesh *meshes,
			 const u16 num_meshes);
void mesh_read(struct mesh *m, FILE *file);
void mesh_write(const struct mesh *m, FILE *file);

#endif /* _MESH_H_ */
