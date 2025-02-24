#include <libdragon.h>
#include <stdio.h>

#include "engine/collision.h"

// #define DEBUG_COLLISION

#if 0
void collision_mesh_free(struct collision_mesh *cm)
{
	free(cm->triangles);
	cm->triangles = NULL;
	cm->triangle_count = 0;
}
#endif /* 0 */
