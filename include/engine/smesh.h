#ifndef ENGINE_SCENE_MESH_H_
#define ENGINE_SCENE_MESH_H_

#include <stdint.h>

#include "engine/vertex.h"
#include "engine/texture.h"

typedef struct {
	uint16_t num_verts, num_indis;
	vertex_t *verts;
	uint16_t *indis;
} smesh_t;

smesh_t *smesh_create_data(uint16_t num_verts, uint16_t num_indis,
		const vertex_t *verts, const uint16_t *indis);
void smesh_destroy(smesh_t *m);
void smesh_draw(const smesh_t *m, const uint32_t tid);

#endif /* ENGINE_SCENE_MESH_H_ */
