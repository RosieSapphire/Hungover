#ifndef _ENGINE_OBJECT_H_
#define _ENGINE_OBJECT_H_

#ifndef IS_USING_SCENE_CONVERTER
#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#else
#define OBJECT_NAME_MAX_LENGTH 32
#endif

typedef struct {
#ifndef IS_USING_SCENE_CONVERTER
	T3DModel *mdl;
	T3DMat4FP *matrix;
	rspq_block_t *displaylist;
#else
	char name[OBJECT_NAME_MAX_LENGTH];
#endif
	T3DVec3 position, position_old, rotation, rotation_old, scale,
		scale_old;
} object_t;

#ifndef IS_USING_SCENE_CONVERTER
object_t object_read_from_file(FILE *file);
/*
object_t object_init_from_model_path(const char *path, const T3DVec3 *pos,
				     const T3DVec3 *rot, const T3DVec3 *scale);
object_t object_init_from_model_pointer(T3DModel *mdl, const T3DVec3 *pos,
					const T3DVec3 *rot,
					const T3DVec3 *scale);
					*/
void object_render(object_t *obj);
rspq_block_t *objects_instanced_gen_dl(const int num_objs, object_t *objs,
				       const T3DModel *common_mdl);
void object_matrix_setup(object_t *o, const float subtick);
void object_terminate(object_t *o, const int should_free_model);
#endif

#endif /* _ENGINE_OBJECT_H_ */
