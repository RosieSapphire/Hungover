#ifndef _ENGINE_MESH_H_
#define _ENGINE_MESH_H_

#include "engine/types.h"
#include "engine/vertex.h"
#include "engine/texture.h"
#include "engine/config.h"

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

struct mesh *mesh_create_data(const char *name, u16 num_verts,
		u16 num_indis, const struct vertex *verts,
		const u16 *indis);
void mesh_copy(const struct mesh *src, struct mesh *dst);
void mesh_destroy(struct mesh *m);
void mesh_draw(const void *sc, const struct mesh *m);
void mesh_draw_tex(const struct mesh *m, const u32 tex);

#endif /* _ENGINE_MESH_H_ */
