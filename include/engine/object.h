#ifndef _ENGINE_OBJECT_H_
#define _ENGINE_OBJECT_H_

#ifndef IS_USING_SCENE_CONVERTER
#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#else
#define OBJECT_NAME_MAX_LENGTH 32
#endif

enum { OBJECT_TYPE_STATIC, OBJECT_TYPE_DOOR, NUM_OBJECT_TYPES };

enum {
	OBJECT_UPDATE_RETURN_NONE,
	OBJECT_UPDATE_RETURN_LOAD_NEXT_AREA,
	OBJECT_UPDATE_RETURN_UNLOAD_PREV_AREA,
	OBJECT_NUM_UPDATE_RETURNS
};

enum {
	OBJECT_FLAG_IS_ACTIVE = (1 << 0),
	OBJECT_FLAG_MUST_REENTER_RADIUS_TO_INTERACT = (1 << 1)
};

#define OBJECT_MAX_NUM_ARGIS 4
#define OBJECT_MAX_NUM_ARGFS 4

enum {
	OBJECT_DOOR_ARGI_NEXT_AREA,
};

enum {
	OBJECT_DOOR_ARGF_SWING_AMOUNT,
	OBJECT_DOOR_ARGF_SWING_AMOUNT_OLD,
};

typedef struct {
#ifndef IS_USING_SCENE_CONVERTER
	T3DModel *mdl;
	T3DMat4FP *matrix;
	rspq_block_t *displaylist;
#else
	char name[OBJECT_NAME_MAX_LENGTH];
#endif
	T3DVec3 position, position_old, position_init, scale, scale_old,
		scale_init;
	T3DQuat rotation, rotation_old, rotation_init;
	uint8_t type;

	/* `arg` means different things depending on the `type`.
	 * Full list of argument types below object struct definition */
	uint8_t argi[OBJECT_MAX_NUM_ARGIS];
	float argf[OBJECT_MAX_NUM_ARGFS];
	uint8_t flags;
} object_t;

/*
 * ARGUMENT TYPES:
 * Static:
 * 	Nothing (so far)
 *
 * Door:
 * 	Next area to load when opened
 */

#ifndef IS_USING_SCENE_CONVERTER
object_t object_read_from_file(FILE *file, const T3DVec3 *offset);
/*
object_t object_init_from_model_path(const char *path, const T3DVec3 *pos,
				     const T3DVec3 *rot, const T3DVec3 *scale);
object_t object_init_from_model_pointer(T3DModel *mdl, const T3DVec3 *pos,
					const T3DVec3 *rot,
					const T3DVec3 *scale);
					*/
int object_update(object_t *obj, const T3DVec3 *player_pos, const float dt);
void object_render(const object_t *obj);
rspq_block_t *objects_instanced_gen_dl(const int num_objs, object_t *objs,
				       const T3DModel *common_mdl);
void object_matrix_setup(object_t *o, const float subtick);
void object_terminate(object_t *o, const int should_free_model);
#endif

#endif /* _ENGINE_OBJECT_H_ */
