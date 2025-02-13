#ifndef _GLB_TO_SCN_EXPORT_H_
#define _GLB_TO_SCN_EXPORT_H_

#ifndef IS_USING_SCENE_CONVERTER
#define IS_USING_SCENE_CONVERTER
#endif /* IS_USING_SCENE_CONVERTER */

#include <stdio.h>

#include "t3d_def.h"
#include "../../include/engine/scene.h"

void scene_export_to_file(const struct scene *scn, FILE *file);
void area_export_to_file(const struct area *a, FILE *file);
void collision_mesh_export_to_file(const struct collision_mesh *cm, FILE *file);
void actor_export_to_file(const struct actor_header *actor, FILE *file);

#endif /* _GLB_TO_SCN_EXPORT_H_ */
