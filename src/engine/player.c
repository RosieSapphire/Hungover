#include "util.h"
#include "t3d_ext.h"
#include "input.h"

#include "t3d_ext.h"

#include "engine/player.h"

#define PLAYER_RADIUS (0.36f * T3DM_TO_N64_SCALE)
#define PLAYER_HEIGHT (1.5748f * T3DM_TO_N64_SCALE)
#define PLAYER_COLLISION_CHECK_Y_OFFSET (0.7874f * T3DM_TO_N64_SCALE)
#define PLAYER_MAX_COLLISION_DIST (69.20f * T3DM_TO_N64_SCALE)

#define PLAYER_TURN_DEG_PER_SEC ((INPUT_GET_BTN(Z, HELD) ? 236.59f : 169.3f))
#define PLAYER_TURN_Y_MAX 89.9f
#define PLAYER_MOVE_UNITS_PER_SEC \
	((INPUT_GET_BTN(Z, HELD) ? 4.4704f : 1.34112f) * T3DM_TO_N64_SCALE)

player_t player_init(collision_mesh_t *colmesh_ptrs, const int num_colmesh_ptrs)
{
	player_t p;

	p.pos = p.pos_old = T3D_VEC3_ZERO;
	p.yaw = p.yaw_old = p.pitch = p.pitch_old = 0.f;
	for (int i = 0; i < num_colmesh_ptrs; i++) {
		p.collision_mesh_ptrs[i] = colmesh_ptrs + i;
	}

	return p;
}

void player_get_look_values(T3DVec3 *eye, T3DVec3 *focus, const player_t *p,
			    const float interp)
{
	/* eye */
	T3DVec3 pos_lerp;
	T3DVec3 eye_offset;

	t3d_vec3_lerp(&pos_lerp, &p->pos_old, &p->pos, interp);
	t3d_vec3_scale(&eye_offset, &T3D_VEC3_UP, PLAYER_HEIGHT);
	t3d_vec3_add(eye, &pos_lerp, &eye_offset);

	/* focus */
	float yaw_lerp = t3d_lerp(p->yaw_old, p->yaw, interp);
	float pitch_lerp = t3d_lerp(p->pitch_old, p->pitch, interp);

	*focus = T3D_VEC3(cosf(T3D_DEG_TO_RAD(yaw_lerp)) *
				  cosf(T3D_DEG_TO_RAD(pitch_lerp)),
			  T3D_DEG_TO_RAD(pitch_lerp),
			  sinf(T3D_DEG_TO_RAD(yaw_lerp)) *
				  cosf(T3D_DEG_TO_RAD(pitch_lerp)));
	t3d_vec3_add(focus, focus, eye);
}

void player_get_focus_pos(T3DVec3 *eye, const player_t *p, const float interp)
{
	T3DVec3 pos_lerp;
	T3DVec3 eye_offset;

	t3d_vec3_lerp(&pos_lerp, &p->pos_old, &p->pos, interp);
	t3d_vec3_scale(&eye_offset, &T3D_VEC3_UP, PLAYER_HEIGHT);
	t3d_vec3_add(eye, &pos_lerp, &eye_offset);
}

static void _player_update_look_angles(player_t *p, const float dt)
{
	float stick[2] = { INPUT_GET_STICK(X), INPUT_GET_STICK(Y) };

	/* yaw */
	p->yaw += stick[0] * PLAYER_TURN_DEG_PER_SEC * dt;
	while (p->yaw >= 360.f) {
		p->yaw -= 360.f;
		p->yaw_old -= 360.f;
	}
	while (p->yaw <= 0.f) {
		p->yaw += 360.f;
		p->yaw_old += 360.f;
	}

	/* pitch */
	p->pitch += stick[1] * LOOK_Y_SIGN * PLAYER_TURN_DEG_PER_SEC * dt;
	if (p->pitch >= PLAYER_TURN_Y_MAX) {
		p->pitch = PLAYER_TURN_Y_MAX;
	}
	if (p->pitch <= -PLAYER_TURN_Y_MAX) {
		p->pitch = -PLAYER_TURN_Y_MAX;
	}
}

static void _player_update_position(player_t *p, const float dt)
{
	T3DVec3 eye, focus, look_dir;
	player_get_look_values(&eye, &focus, p, 1.f);
	t3d_vec3_diff(&look_dir, &focus, &eye);

	const int forw_sign =
		INPUT_GET_BTN(C_UP, HELD) - INPUT_GET_BTN(C_DOWN, HELD);
	const int side_sign =
		INPUT_GET_BTN(C_RIGHT, HELD) - INPUT_GET_BTN(C_LEFT, HELD);
	T3DVec3 forw_move = look_dir;
	forw_move.v[1] = 0.f;
	t3d_vec3_norm(&forw_move);
	T3DVec3 side_move, move;
	t3d_vec3_cross(&side_move, &forw_move, &T3D_VEC3_UP);
	t3d_vec3_scale(&forw_move, &forw_move, forw_sign);
	t3d_vec3_scale(&side_move, &side_move, side_sign);

	t3d_vec3_add(&move, &forw_move, &side_move);
	t3d_vec3_norm(&move);
	t3d_vec3_scale(&move, &move, PLAYER_MOVE_UNITS_PER_SEC * dt);

	t3d_vec3_add(&p->pos, &p->pos, &move);
}

static void _player_update_collision(player_t *p, const uint16_t col_index)
{
	collision_mesh_t *cm = p->collision_mesh_ptrs[col_index];

	for (uint16_t i = 0; i < cm->num_triangles; i++) {
		collision_triangle_t *tri = cm->triangles + i;
		T3DVec3 dir;
		T3DVec3 tri_verts[3];
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				tri_verts[j].v[k] =
					tri->verts[j].pos[k] + cm->offset.v[k];
			}
		}
		T3DVec3 tri_norm = *(T3DVec3 *)tri->norm;

		t3d_vec3_negate(&dir, &tri_norm);

		float dist;
		T3DVec3 check_offset;
		T3DVec3 check_pos;

		t3d_vec3_scale(&check_offset, &T3D_VEC3_UP,
			       PLAYER_COLLISION_CHECK_Y_OFFSET);
		t3d_vec3_add(&check_pos, &p->pos, &check_offset);

		if (!t3d_raycast_triangle(&check_pos, &dir, tri_verts, &dist)) {
			continue;
		}

		if (dist > PLAYER_MAX_COLLISION_DIST) {
			continue;
		}

		T3DVec3 push_vec;
		float push_amnt = 0.f;

		const float tri_dot = t3d_vec3_dot(&tri_norm, &T3D_VEC3_UP);
		if (!tri_dot) { /* is wall */
			push_amnt = fmaxf(PLAYER_RADIUS - dist, 0.f);
		} else if (tri_dot > 0.4f) { /* is floor */
			push_amnt = -dist + PLAYER_COLLISION_CHECK_Y_OFFSET;
		}

		t3d_vec3_scale(&push_vec, &tri_norm, push_amnt);
		t3d_vec3_add(&p->pos, &push_vec, &p->pos);
	}

	// debugf("\n\n");
}

void player_update(player_t *p, const scene_t *scn, const float dt)
{
	/* old values */
	p->yaw_old = p->yaw;
	p->pitch_old = p->pitch;
	p->pos_old = p->pos;

	/* update collision parameters */
	p->collision_mesh_ptrs[0] = &scn->areas[scn->area_index].colmesh;
	p->collision_mesh_ptrs[1] =
		(scn->flags & SCENE_FLAG_PROCESS_AREA_LAST) ?
			&scn->areas[scn->area_index_old].colmesh :
			NULL;

	_player_update_look_angles(p, dt);
	_player_update_position(p, dt);

	/* collision with static geometry */
	for (int i = 0; i < 2; i++) {
		if (p->collision_mesh_ptrs[i]) {
			_player_update_collision(p, i);
		}
	}

	/* collision with objects */
	/*
	area_t *area = scn->areas + scn->area_index;
	for (uint16_t i = 0; i < area->num_objects; i++) {
		object_update(area->objects + i, &p->pos, dt);
	}
	*/
}

void player_to_viewport(T3DViewport *vp, const player_t *p, const float interp)
{
	T3DVec3 eye, focus;

	player_get_look_values(&eye, &focus, p, interp);
	t3d_viewport_look_at(vp, &eye, &focus, &T3D_VEC3_UP);
}

void player_terminate(player_t *p)
{
	p->collision_mesh_ptrs[0] = p->collision_mesh_ptrs[1] = NULL;
	p->yaw = p->yaw_old = p->pitch = p->pitch_old = 0.f;
	p->pos = p->pos_old = T3D_VEC3_ZERO;
}
