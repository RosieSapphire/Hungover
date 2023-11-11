#ifndef _ENGINE_SCENE_MESH_H_
#define _ENGINE_SCENE_MESH_H_

#include "engine/types.h"
#include "engine/vertex.h"
#include "engine/texture.h"
#include "engine/config.h"

typedef struct
{
	char name[CONF_NAME_MAX_LEN];
	u16 num_verts, num_indis;
	vertex_t *verts;
	u16 *indis;
	u16 tex_index;
} smesh_t;

smesh_t *smesh_create_data(const char *name, u16 num_verts,
		u16 num_indis, const vertex_t *verts,
		const u16 *indis);
void smesh_copy(const smesh_t *src, smesh_t *dst);
void smesh_destroy(smesh_t *m);
void smesh_draw(const void *sc, const smesh_t *m);
void smesh_draw_tex(const smesh_t *m, const u32 tex);

#endif /* _ENGINE_SCENE_MESH_H_ */
