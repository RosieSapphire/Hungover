#ifndef _ENGINE_SCENE_H_
#define _ENGINE_SCENE_H_

#ifdef IS_USING_GLTF_TO_SCN
#include "../../../include/types.h"

#include "../../../include/engine/area.h"
#else
#include "types.h"

#include "engine/area.h"
#endif

enum { SCENE_FLAG_PROCESS_AREA_LAST = (1 << 0) };

struct scene {
	u16 area_count;
	struct area *areas;
#ifndef IS_USING_GLTF_TO_SCN
	T3DModel *mdl;
	u16 area_index;
	u16 area_index_old;
	u8 flags;
#endif
};

struct scene scene_init_from_file(const char *path);
void scene_update(struct scene *scn, const T3DVec3 *player_pos,
		  const T3DVec3 *player_dir, const f32 dt);
void scene_render(const struct scene *scn, const f32 subtick);
void scene_free(struct scene *scn);

#endif /* _ENGINE_SCENE_H_ */
