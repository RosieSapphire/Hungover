#ifndef _GLB_TO_SCN_UTIL_H_
#define _GLB_TO_SCN_UTIL_H_

#ifndef IS_USING_SCENE_CONVERTER
#define IS_USING_SCENE_CONVERTER
#endif /* IS_USING_SCENE_CONVERTER */

#include "t3d_def.h"
#include "../../include/engine/scene.h"

void exitf(const char *fmt, ...);
void triangle_calc_normal(struct collision_triangle *tri);
void quaternion_from_matrix(float quat[4], float matrix[4][4]);
void scene_debug(const struct scene *s, const char *scene_path);

#endif /* _GLB_TO_SCN_UTIL_H_ */
