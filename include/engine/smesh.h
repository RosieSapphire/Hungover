#ifndef _ENGINE_SCENE_MESH_H_
#define _ENGINE_SCENE_MESH_H_

#include "engine/types.h"
#include "engine/vertex.h"
#include "engine/texture.h"
#include "engine/config.h"

/**
 * smesh - Mesh Struct
 * @name: Mesh Name
 * @num_verts: Number of Vertices
 * @num_indis: Number of Indices
 * @verts: Vertices Array
 * @indis: Indices Array
 * @tex_index: Texture Index
 */
typedef struct
{
	char name[CONF_NAME_MAX_LEN];
	u16 num_verts, num_indis;
	struct vertex *verts;
	u16 *indis;
	u16 tex_index;
} smesh_t;

smesh_t *smesh_create_data(const char *name, u16 num_verts,
		u16 num_indis, const struct vertex *verts,
		const u16 *indis);
void smesh_copy(const smesh_t *src, smesh_t *dst);
void smesh_destroy(smesh_t *m);
void smesh_draw(const void *sc, const smesh_t *m);
void smesh_draw_tex(const smesh_t *m, const u32 tex);

#endif /* _ENGINE_SCENE_MESH_H_ */
