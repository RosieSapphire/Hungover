#include "t3d_ext.h"

#include "engine/object.h"

#define OBJECT_NAME_MAX_LENGTH 32

#define OBJECT_DOOR_TURN_DEG_PER_SEC 179.f
#define OBJECT_DOOR_CHECK_RADIUS 64.f

object_t object_read_from_file(FILE *file, const T3DVec3 *offset)
{
	object_t o;
	char obj_name[OBJECT_NAME_MAX_LENGTH];
	char obj_name_new[OBJECT_NAME_MAX_LENGTH];
	char mdl_path[OBJECT_NAME_MAX_LENGTH << 1];
	memset(obj_name, 0, OBJECT_NAME_MAX_LENGTH);
	memset(obj_name_new, 0, OBJECT_NAME_MAX_LENGTH);
	memset(mdl_path, 0, OBJECT_NAME_MAX_LENGTH << 1);

	fread(obj_name, 1, OBJECT_NAME_MAX_LENGTH, file);
	long int obj_val;
	{
		const char *last_dot_str = strrchr(obj_name, '.') + 1;
		const int last_dot_str_len = strlen(last_dot_str) + 1;

		char *endptr;
		obj_val = strtol(last_dot_str, &endptr, 10);
		const int has_num = endptr != (strrchr(obj_name, '.') + 1);
		if (has_num) {
			strncpy(obj_name_new, obj_name,
				strlen(obj_name) - last_dot_str_len);
		} else {
			strncpy(obj_name_new, obj_name, OBJECT_NAME_MAX_LENGTH);
		}
		/*
		debugf("'%s' -> '%s'", obj_name, obj_name_new);
		if (has_num) {
			debugf(" (%ld)\n", obj_val);
		} else {
			debugf("\n");
		}
		debugf("\n");
		*/
	}

	snprintf(mdl_path, OBJECT_NAME_MAX_LENGTH << 1, "rom:/%s.t3dm",
		 obj_name_new);

	o.mdl = t3d_model_load(mdl_path);
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
	o.position = o.position_old = o.position_init = pos;
	o.rotation = o.rotation_old = o.rotation_init = rot;
	o.scale = o.scale_old = o.scale_init = scale;

	/* determining type */
	if (!strncmp("Door", obj_name + 4, strlen("Door"))) {
		o.type = OBJECT_TYPE_DOOR;
		o.argi[OBJECT_DOOR_ARGI_NEXT_AREA] = obj_val;
		o.argf[OBJECT_DOOR_ARGF_SWING_AMOUNT] = 0.f;
		o.argf[OBJECT_DOOR_ARGF_SWING_AMOUNT_OLD] = 0.f;
	} else {
		o.type = OBJECT_TYPE_STATIC;
		o.argi[OBJECT_DOOR_ARGI_NEXT_AREA] = 0;
	}

	o.flags = OBJECT_FLAG_IS_ACTIVE;

	return o;
}

static int _object_update_door(object_t *obj, const float dist_from_player,
			       const float dt)
{
	obj->rotation_old = obj->rotation;
	obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT_OLD] =
		obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT];

	if (dist_from_player < OBJECT_DOOR_CHECK_RADIUS) {
		if (!(obj->flags &
		      OBJECT_FLAG_MUST_REENTER_RADIUS_TO_INTERACT)) {
			obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT] +=
				dt * OBJECT_DOOR_TURN_DEG_PER_SEC;
			if (obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT] > 90.f) {
				obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT] = 90.f;
			}
		}
	} else {
		obj->flags &= ~(OBJECT_FLAG_MUST_REENTER_RADIUS_TO_INTERACT);
		obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT] -=
			dt * OBJECT_DOOR_TURN_DEG_PER_SEC;
		if (obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT] < 0.f) {
			obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT] = 0.f;
		}
	}

	obj->rotation = obj->rotation_init;
	t3d_quat_rotate_euler(
		&obj->rotation, (float[3]){ 0, 0, 1 },
		T3D_DEG_TO_RAD(obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT]));
	// obj->rotation.v[2] = obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT];

	if (obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT] > 0.f &&
	    obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT_OLD] <= 0.f) {
		return OBJECT_UPDATE_RETURN_LOAD_NEXT_AREA;
	}

	if (obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT] <= 0.f &&
	    obj->argf[OBJECT_DOOR_ARGF_SWING_AMOUNT_OLD] > 0.f) {
		return OBJECT_UPDATE_RETURN_UNLOAD_PREV_AREA;
	}

	return OBJECT_UPDATE_RETURN_NONE;
}

int object_update(object_t *obj, const T3DVec3 *player_pos, const float dt)
{
	/* This should never really occur, since we check for this in the
	 * scene update, but it's here if this function were to be called
	 * somewhere else, for example. */
	if (!(obj->flags & OBJECT_FLAG_IS_ACTIVE)) {
		return OBJECT_UPDATE_RETURN_NONE;
	}

	T3DVec3 obj_vec;
	t3d_vec3_diff(&obj_vec, &obj->position, player_pos);
	const float obj_dist = t3d_vec3_len(&obj_vec);

	switch (obj->type) {
	case OBJECT_TYPE_DOOR:
		return _object_update_door(obj, obj_dist, dt);

	case OBJECT_TYPE_STATIC:
	default:
		return OBJECT_UPDATE_RETURN_NONE;
	}

	return OBJECT_UPDATE_RETURN_NONE;
}

void object_render(const object_t *obj)
{
	if (!(obj->flags & OBJECT_FLAG_IS_ACTIVE)) {
		return;
	}

	rspq_block_run(obj->displaylist);
}

rspq_block_t *objects_instanced_gen_dl(const int num_objs, object_t *objs,
				       const T3DModel *common_mdl)
{
	rspq_block_t *instdl = NULL;

	rspq_block_begin();
	for (int i = 0; i < num_objs; i++) {
		object_t *o = objs + i;
		assertf(o->mdl == common_mdl,
			"Object %d in array of %d does not share "
			"a common model, and cannot be instanced\n",
			i, num_objs);
		object_render(o);
	}
	return (instdl = rspq_block_end());
}

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

void object_matrix_setup(object_t *o, const float subtick)
{
	T3DVec3 pos_lerp, scale_lerp;
	T3DQuat rot_lerp;

	t3d_vec3_lerp(&pos_lerp, &o->position_old, &o->position, subtick);
	_quat_slerp(&rot_lerp, &o->rotation_old, &o->rotation, subtick);
	/* TODO: Clamp rotation */
	t3d_vec3_lerp(&scale_lerp, &o->scale_old, &o->scale, subtick);

	t3d_mat4fp_from_srt(o->matrix, scale_lerp.v, rot_lerp.v, pos_lerp.v);
}

void object_terminate(object_t *o, const int should_free_model)
{
	o->position = o->position_old = o->scale = o->scale_old = T3D_VEC3_ZERO;
	o->rotation = o->rotation_old = T3D_QUAT_IDENTITY;

	free_uncached(o->matrix);
	o->matrix = NULL;

	rspq_block_free(o->displaylist);
	o->displaylist = NULL;

	if (should_free_model) {
		t3d_model_free(o->mdl);
	}
	o->mdl = NULL;
}
