#include "t3d_ext.h"
#include "input.h"

#include "engine/ui.h"
#include "engine/object_door.h"
#include "engine/object.h"

#define OBJECT_NAME_MAX_LENGTH 32

Object objectInitFromFile(FILE *file, const T3DVec3 *offset,
			  const int areaIndex)
{
	Object o;
	char objName[OBJECT_NAME_MAX_LENGTH];
	char objNameNew[OBJECT_NAME_MAX_LENGTH];
	char mdlPath[OBJECT_NAME_MAX_LENGTH << 1];
	memset(objName, 0, OBJECT_NAME_MAX_LENGTH);
	memset(objNameNew, 0, OBJECT_NAME_MAX_LENGTH);
	memset(mdlPath, 0, OBJECT_NAME_MAX_LENGTH << 1);

	fread(objName, 1, OBJECT_NAME_MAX_LENGTH, file);
	long int objVal;
	{
		const char *lastDotStr = strrchr(objName, '.') + 1;
		const int lastDotStrLen = strlen(lastDotStr) + 1;

		char *endptr;
		objVal = strtol(lastDotStr, &endptr, 10);
		const int hasNum = endptr != (strrchr(objName, '.') + 1);
		if (hasNum) {
			strncpy(objNameNew, objName,
				strlen(objName) - lastDotStrLen);
		} else {
			strncpy(objNameNew, objName, OBJECT_NAME_MAX_LENGTH);
		}
	}

	snprintf(mdlPath, OBJECT_NAME_MAX_LENGTH << 1, "rom:/%s.t3dm",
		 objNameNew);

	o.mdl = t3d_model_load(mdlPath);
	o.matrix = malloc_uncached(sizeof *o.matrix);

	rspq_block_begin();
	t3d_matrix_push(o.matrix);
	t3d_model_draw(o.mdl);
	t3d_matrix_pop(1);
	o.displaylist = rspq_block_end();

	T3DVec3 pos, scale;
	T3DQuat rot;

	for (int i = 0; i < 3; i++) {
		fread(pos.v + i, 4, 1, file);
	}

	for (int i = 0; i < 4; i++) {
		fread(rot.v + i, 4, 1, file);
	}

	for (int i = 0; i < 3; i++) {
		fread(scale.v + i, 4, 1, file);
	}

	t3d_vec3_add(&pos, &pos, offset);
	o.position = o.positionOld = o.positionInit = pos;
	o.rotation = o.rotationOld = o.rotationInit = rot;
	o.scale = o.scaleOld = o.scaleInit = scale;

	/* determining type */
	if (!strncmp("Door", objName + 4, strlen("Door"))) {
		o.type = OBJECT_TYPE_DOOR;
		o.subobjectIndex = doorObjectInit(objVal, areaIndex);
	} else {
		o.type = OBJECT_TYPE_STATIC;
	}

	o.flags = OBJECT_FLAG_IS_ACTIVE;

	return o;
}

void objectSetupFrameStaticVars(void)
{
	numDoorsInRangeOf = 0;
}

void objectUpdateUIWithStaticVars(void)
{
	uiToggleElements(UI_ELEMENT_FLAG_A_BUTTON, numDoorsInRangeOf);
}

int objectUpdate(Object *obj, const T3DVec3 *playerPos,
		 const T3DVec3 *playerDir, const float dt)
{
	/* This should never really occur, since we check for this in the
	 * scene update, but it's here if this function were to be called
	 * somewhere else, for example. */
	if (!(obj->flags & OBJECT_FLAG_IS_ACTIVE) ||
	    (obj->flags & OBJECT_FLAG_WAS_UPDATED_THIS_FRAME)) {
		return OBJECT_UPDATE_RETURN_NONE;
	}

	obj->flags |= OBJECT_FLAG_WAS_UPDATED_THIS_FRAME;

	T3DVec3 objVec, objDir;
	t3d_vec3_diff(&objVec, &obj->position, playerPos);
	const float objDist = t3d_vec3_len(&objVec);
	t3d_vec3_scale(&objDir, &objVec, 1.f / objDist);
	const float playerObjDot = t3d_vec3_dot(playerDir, &objDir);
	int ret = OBJECT_UPDATE_RETURN_NONE;

	switch (obj->type) {
	case OBJECT_TYPE_DOOR:
		DoorObject *door = doorObjects + obj->subobjectIndex;
		obj->rotationOld = obj->rotation;
		ret = doorObjectUpdate(door, &objDir, objDist, playerObjDot,
				       dt);
		obj->rotation = obj->rotationInit;
		t3d_quat_rotate_euler(&obj->rotation, (float[3]){ 0, 0, 1 },
				      T3D_DEG_TO_RAD(door->swingAmount));

		break;

	case OBJECT_TYPE_STATIC:
	default:
		ret = OBJECT_UPDATE_RETURN_NONE;
		break;
	}

	return ret;
}

void objectRender(const Object *obj)
{
	if (!(obj->flags & OBJECT_FLAG_IS_ACTIVE)) {
		return;
	}

	rspq_block_run(obj->displaylist);
}

rspq_block_t *objectsInstancedGenDL(const int numObjs, Object *objs,
				    const T3DModel *commonModel)
{
	rspq_block_t *instdl = NULL;

	rspq_block_begin();
	for (int i = 0; i < numObjs; i++) {
		Object *o = objs + i;
		assertf(o->mdl == commonModel,
			"Object %d in array of %d does not share "
			"a common model, and cannot be instanced\n",
			i, numObjs);
		objectRender(o);
	}
	return (instdl = rspq_block_end());
}

/*
static void _quat_slerp(T3DQuat *res, const T3DQuat *a, const T3DQuat *b,
			const float t)
{
	T3DQuat q1, q2;
	float cos_theta, sin_theta, angle;

	cos_theta = t3d_quat_dot(a, b);
	q1 = *a;

	if (fabsf(cos_theta) >= 1.f) {
		*res = q1;
		return;
	}

	if (cos_theta < 0.0f) {
		t3d_quat_negate(&q1, &q1);
		cos_theta = -cos_theta;
	}

	sin_theta = sqrtf(1.f - cos_theta * cos_theta);

	if (fabsf(sin_theta) < 0.001f) {
		t3d_quat_nlerp(res, a, b, t);
		return;
	}

	angle = acosf(cos_theta);
	t3d_quat_scale(&q1, &q1, sinf((1.f - t) * angle));
	t3d_quat_scale(&q2, b, sinf(t * angle));

	t3d_quat_add(&q1, &q1, &q2);
	t3d_quat_scale(res, &q1, 1.f / sin_theta);
}
*/

void objectMatrixSetup(Object *o, const float subtick)
{
	T3DVec3 pos_lerp, scale_lerp;
	T3DQuat rot_lerp;

	t3d_vec3_lerp(&pos_lerp, &o->positionOld, &o->position, subtick);
	// t3d_quat_slerp(&rot_lerp, &o->rotationOld, &o->rotation, subtick);
	t3d_quat_nlerp(&rot_lerp, &o->rotationOld, &o->rotation, subtick);
	t3d_vec3_lerp(&scale_lerp, &o->scaleOld, &o->scale, subtick);

	t3d_mat4fp_from_srt(o->matrix, scale_lerp.v, rot_lerp.v, pos_lerp.v);
}

void objectFree(Object *o, const int shouldFreeModel)
{
	o->position = o->positionOld = o->scale = o->scaleOld = T3D_VEC3_ZERO;
	o->rotation = o->rotationOld = T3D_QUAT_IDENTITY;

	free_uncached(o->matrix);
	o->matrix = NULL;

	rspq_block_free(o->displaylist);
	o->displaylist = NULL;

	if (shouldFreeModel) {
		t3d_model_free(o->mdl);
	}
	o->mdl = NULL;
}
