#ifndef ENGINE_SCENE_MESH_H_
#define ENGINE_SCENE_MESH_H_

#include <stdint.h>

#include "engine/vertex.h"
#include "engine/texture.h"
#include "engine/config.h"

typedef struct {
	char name[CONF_NAME_MAX_LEN];
	uint16_t num_verts, num_indis;
	vertex_t *verts;
	uint16_t *indis;
	uint16_t tex_index;
} smesh_t;

smesh_t *smesh_create_data(const char *name, uint16_t num_verts,
		uint16_t num_indis, const vertex_t *verts,
		const uint16_t *indis);
void smesh_copy(const smesh_t *src, smesh_t *dst);
void smesh_destroy(smesh_t *m);
void smesh_draw(const void *sc, const smesh_t *m);
void smesh_draw_tex(const smesh_t *m, const uint32_t tex);

#endif /* ENGINE_SCENE_MESH_H_ */
