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
	OBJECT_UPDATE_RETURN_UNLOAD_NEXT_AREA,
	OBJECT_NUM_UPDATE_RETURNS
};

enum {
	OBJECT_FLAG_IS_ACTIVE = (1 << 0),
	OBJECT_FLAG_MUST_REENTER_RADIUS = (1 << 1),
	OBJECT_FLAG_WAS_UPDATED_THIS_FRAME = (1 << 2)
};

#define OBJECT_MAX_NUM_ARGIS 4
#define OBJECT_MAX_NUM_ARGFS 4

enum {
	OBJECT_DOOR_ARGI_NEXT_AREA,
	OBJECT_DOOR_ARGI_IS_OPENING,
	OBJECT_DOOR_ARGI_SIDE_ENTERED,
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
	T3DVec3 position, positionOld, positionInit, scale, scaleOld, scaleInit;
	T3DQuat rotation, rotationOld, rotationInit;
	uint8_t type;

	/* FIXME: Defer this to a specific object type per object */
	/* `arg` means different things depending on the `type`.
	 * Full list of argument types below object struct definition */
	uint8_t argi[OBJECT_MAX_NUM_ARGIS];
	float argf[OBJECT_MAX_NUM_ARGFS];
	uint8_t flags;
} Object;

/*
 * ARGUMENT TYPES:
 * Static:
 * 	Nothing (so far)
 *
 * Door:
 * 	Next area to load when opened
 */

#ifndef IS_USING_SCENE_CONVERTER
Object objectInitFromFile(FILE *file, const T3DVec3 *offset);
void objectSetupFrameStaticVars(void);
void objectUpdateUIWithStaticVars(void);
int objectUpdate(Object *obj, const T3DVec3 *playerPos,
		 const T3DVec3 *playerDir, const float dt);
void objectRender(const Object *obj);
rspq_block_t *objectsInstancedGenDL(const int numObjs, Object *objs,
				    const T3DModel *commonModel);
void objectMatrixSetup(Object *o, const float subtick);
void objectFree(Object *o, const int shouldFreeModel);
#endif

#endif /* _ENGINE_OBJECT_H_ */
