#include "input.h"
#include "t3d_ext.h"

#include "engine/ui.h"
#include "engine/actor_static.h"
#include "engine/actor_door.h"
#include "engine/actor_microwave.h"
#include "engine/actor_pickup.h"

#include "engine/actor.h"

#define ACTOR_NAME_MAX_LENGTH 32

static u8 (*actor_update_funcs[ACTOR_TYPE_COUNT])(
	const u8, const struct actor_update_params *) = {
	actor_static_update,
	actor_door_update,
	actor_microwave_update,
	actor_pickup_update,
};

void actor_static_vars_setup(void)
{
	actor_static_count_in_range = 0;
	actor_door_count_in_range = 0;
	actor_microwave_count_in_range = 0;
	actor_pickup_count_in_range = 0;
}

void actor_static_vars_to_ui(void)
{
	boolean a_button_prompt_visible = 0;

	a_button_prompt_visible += actor_door_count_in_range;
	a_button_prompt_visible += actor_microwave_count_in_range;
	a_button_prompt_visible += actor_pickup_count_in_range;

	ui_elements_toggle(UI_ELEMENT_FLAG_A_BUTTON, a_button_prompt_visible);
}

u8 actor_update(struct actor_header *act, const T3DVec3 *player_pos,
		const T3DVec3 *player_dir, const f32 dt)
{
	/* This should never really occur, since we check for this in the
	 * scene update, but it's here if this function were to be called
	 * somewhere else, for example. */
	if (!(act->flags & ACTOR_FLAG_IS_ACTIVE) ||
	    (act->flags & ACTOR_FLAG_WAS_UPDATED_THIS_FRAME)) {
		return ACTOR_RETURN_NONE;
	}

	act->flags |= ACTOR_FLAG_WAS_UPDATED_THIS_FRAME;

	T3DVec3 act_vec, act_dir;
	t3d_vec3_diff(&act_vec, &act->position, player_pos);
	const f32 act_dist = t3d_vec3_len(&act_vec);
	t3d_vec3_scale(&act_dir, &act_vec, 1.f / act_dist);

	struct actor_update_params params = { .player_to_actor_dir = &act_dir,
					      .player_dir = player_dir,
					      .player_pos = player_pos,
					      .player_dist = act_dist,
					      .dt = dt };

	act->position_old = act->position;
	act->rotation_old = act->rotation;
	act->scale_old = act->scale;

	assertf(act->type < ACTOR_TYPE_COUNT, "Invalid Actor Type (%d)\n",
		act->type);
	return (*actor_update_funcs[act->type])(act->type_index, &params);
}

void actor_render(const struct actor_header *act)
{
	if (!(act->flags & ACTOR_FLAG_IS_ACTIVE)) {
		return;
	}

	rspq_block_run(act->displaylist);
}

/*
rspq_block_t *actorsInstancedGenDL(const s32 numstruct actor_headers,
				   struct actor_header *acts,
				   const T3DModel *commonModel)
{
	rspq_block_t *instdl = NULL;

	rspq_block_begin();
	for (s32 i = 0; i < numstruct actor_headers; i++) {
		struct actor_header *act = acts + i;
		assertf(act->mdl == commonModel,
			"struct actor_header %d in array of %d does not share "
			"a common model, and cannot be instanced\n",
			i, numstruct actor_headers);
		actorRender(act);
	}
	return (instdl = rspq_block_end());
}
*/

void actor_matrix_setup(struct actor_header *act, const f32 subtick)
{
	T3DVec3 pos_lerp, scale_lerp;
	T3DQuat rot_lerp;

	t3d_vec3_lerp(&pos_lerp, &act->position_old, &act->position, subtick);
	t3d_quat_nlerp(&rot_lerp, &act->rotation_old, &act->rotation, subtick);
	t3d_vec3_lerp(&scale_lerp, &act->scale_old, &act->scale, subtick);

	t3d_mat4fp_from_srt(act->matrix, scale_lerp.v, rot_lerp.v, pos_lerp.v);
}

void actor_free(struct actor_header *act, const boolean should_free_model)
{
	act->position = T3D_VEC3_ZERO;
	act->position_old = T3D_VEC3_ZERO;
	act->scale = T3D_VEC3_ZERO;
	act->scale_old = T3D_VEC3_ZERO;
	act->rotation = T3D_QUAT_IDENTITY;
	act->rotation_old = T3D_QUAT_IDENTITY;

	free_uncached(act->matrix);
	act->matrix = NULL;

	rspq_block_free(act->displaylist);
	act->displaylist = NULL;

	if (should_free_model) {
		t3d_model_free(act->mdl);
	}
	act->mdl = NULL;
}
