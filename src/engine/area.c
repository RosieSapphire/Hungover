#include "t3d_ext.h"

#include "engine/area.h"

void area_free(struct area *a);

void area_render(const struct area *a, const f32 subtick)
{
	/* actors */
	for (u16 i = 0; i < a->actor_header_count; i++) {
		struct actor_header *ah = a->actor_headers[i];

		if (!(ah->flags & ACTOR_FLAG_IS_ACTIVE)) {
			continue;
		}

		actor_matrix_setup(ah, subtick);
		actor_render(ah);
	}

	/* static geometry */
	rspq_block_run(a->displaylist);
}

void area_free(struct area *a)
{
	free_uncached(a->matrix);
	a->matrix = NULL;
	for (u16 i = 0; i < a->actor_header_count; i++) {
		actor_free(a->actor_headers[i], true);
	}
	free(a->actor_headers);
	a->actor_headers = NULL;
	a->actor_header_count = 0;
	collision_mesh_free(&a->colmesh);
	a->offset = T3D_VEC3_ZERO;
}
