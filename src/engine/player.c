#include "input.h"
#include "t3d_ext.h"
#include "util.h"

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

#define PLAYER_ENABLE_COLLISION 1

struct player player;

void player_init(void)
{
	player.pos = T3D_VEC3_ZERO;
	player.pos_old = T3D_VEC3_ZERO;
	player.yaw = 0.f;
	player.yaw_old = 0.f;
	player.pitch = 0.f;
	player.pitch_old = 0.f;
	player.colmesh_ptrs[0] = NULL;
	player.colmesh_ptrs[1] = NULL;
	player.inventory = inventory_init();
	player.inv_ent_cur = 0;
}

void player_look_values_get(T3DVec3 *eye, T3DVec3 *focus, const f32 interp)
{
	/* eye */
	T3DVec3 pos_lerp;
	T3DVec3 eye_offset;

	t3d_vec3_lerp(&pos_lerp, &player.pos_old, &player.pos, interp);
	t3d_vec3_scale(&eye_offset, &T3D_VEC3_ZUP, PLAYER_HEIGHT);
	t3d_vec3_add(eye, &pos_lerp, &eye_offset);

	/* focus */
	f32 yaw_lerp = t3d_lerp(player.yaw_old, player.yaw, interp);
	f32 pitch_lerp = t3d_lerp(player.pitch_old, player.pitch, interp);

	*focus = T3D_VEC3(cosf(T3D_DEG_TO_RAD(yaw_lerp)) *
				  cosf(T3D_DEG_TO_RAD(pitch_lerp)),
			  sinf(T3D_DEG_TO_RAD(yaw_lerp)) *
				  cosf(T3D_DEG_TO_RAD(pitch_lerp)),
			  T3D_DEG_TO_RAD(pitch_lerp));
	t3d_vec3_add(focus, focus, eye);
}

static void _player_look_angles_update(const f32 dt)
{
	const f32 stick[2] = { INPUT_GET_STICK(X), INPUT_GET_STICK(Y) };

	/* yaw */
	player.yaw -= stick[0] * PLAYER_TURN_DEG_PER_SEC * dt;
	while (player.yaw >= 360.f) {
		player.yaw -= 360.f;
		player.yaw_old -= 360.f;
	}
	while (player.yaw <= 0.f) {
		player.yaw += 360.f;
		player.yaw_old += 360.f;
	}

	/* pitch */
	player.pitch += stick[1] * LOOK_Y_SIGN * PLAYER_TURN_DEG_PER_SEC * dt;
	if (player.pitch >= PLAYER_TURN_Y_MAX) {
		player.pitch = PLAYER_TURN_Y_MAX;
	}
	if (player.pitch <= -PLAYER_TURN_Y_MAX) {
		player.pitch = -PLAYER_TURN_Y_MAX;
	}
}

static void _player_positions_update(const f32 dt)
{
	T3DVec3 eye, focus, look_dir;
	player_look_values_get(&eye, &focus, 1.f);
	t3d_vec3_diff(&look_dir, &focus, &eye);

	const s8 forw_sign =
		INPUT_GET_BTN(C_UP, HELD) - INPUT_GET_BTN(C_DOWN, HELD);
	const s8 side_sign =
		INPUT_GET_BTN(C_RIGHT, HELD) - INPUT_GET_BTN(C_LEFT, HELD);
	T3DVec3 forw_move = look_dir;
	forw_move.v[2] = 0.f;
	t3d_vec3_norm(&forw_move);
	T3DVec3 sideMove, move;
	t3d_vec3_cross(&sideMove, &forw_move, &T3D_VEC3_ZUP);
	t3d_vec3_scale(&forw_move, &forw_move, forw_sign);
	t3d_vec3_scale(&sideMove, &sideMove, side_sign);

	t3d_vec3_add(&move, &forw_move, &sideMove);
	t3d_vec3_norm(&move);
	t3d_vec3_scale(&move, &move, PLAYER_MOVE_UNITS_PER_SEC * dt);

	t3d_vec3_add(&player.pos, &player.pos, &move);
}

static void _player_collision_update(const u16 col_index)
{
	struct collision_mesh *cm = player.colmesh_ptrs[col_index];

	for (u16 i = 0; i < cm->triangle_count; i++) {
		struct collision_triangle *tri = cm->triangles + i;
		T3DVec3 dir;
		T3DVec3 tri_verts[3];
		for (u8 j = 0; j < 3; j++) {
			for (u8 k = 0; k < 3; k++) {
				tri_verts[j].v[k] =
					tri->verts[j].pos[k] + cm->offset.v[k];
			}
		}
		T3DVec3 triNorm = *(T3DVec3 *)tri->norm;

		t3d_vec3_negate(&dir, &triNorm);

		f32 dist;
		T3DVec3 check_offset;
		T3DVec3 check_pos;

		t3d_vec3_scale(&check_offset, &T3D_VEC3_ZUP,
			       PLAYER_COLLISION_CHECK_Y_OFFSET);
		t3d_vec3_add(&check_pos, &player.pos, &check_offset);

		if (!t3d_raycast_triangle(&check_pos, &dir, tri_verts, &dist)) {
			continue;
		}

		if (dist > PLAYER_MAX_COLLISION_DIST) {
			continue;
		}

		T3DVec3 push_vec;
		f32 push_amnt = 0.f;
		const f32 tri_dot = t3d_vec3_dot(&triNorm, &T3D_VEC3_ZUP);
		if (!tri_dot) { /* is wall */
			push_amnt = fmaxf(PLAYER_RADIUS - dist, 0.f);
		} else if (tri_dot > 0.4f) { /* is floor */
			push_amnt = -dist + PLAYER_COLLISION_CHECK_Y_OFFSET;
		}

		t3d_vec3_scale(&push_vec, &triNorm, push_amnt);
		t3d_vec3_add(&player.pos, &push_vec, &player.pos);
	}
}

void player_update(const struct scene *scn, const f32 dt)
{
	/* old values */
	player.yaw_old = player.yaw;
	player.pitch_old = player.pitch;
	player.pos_old = player.pos;

#if PLAYER_ENABLE_COLLISION
	player.colmesh_ptrs[0] = &scn->areas[scn->area_index].colmesh;
	player.colmesh_ptrs[1] =
		(scn->flags & SCENE_FLAG_PROCESS_AREA_LAST) ?
			&scn->areas[scn->area_index_old].colmesh :
			NULL;
#else
	player.colmesh_ptrs[0] = NULL;
	player.colmesh_ptrs[1] = NULL;
#endif

	_player_look_angles_update(dt);
	_player_positions_update(dt);

	for (u8 i = 0; i < 2; i++) {
		if (player.colmesh_ptrs[i]) {
			_player_collision_update(i);
		}
	}

	/* TODO: Collision with objects */
}

void player_to_viewport(T3DViewport *vp, const f32 interp)
{
	T3DVec3 eye, focus;

	player_look_values_get(&eye, &focus, interp);
	t3d_viewport_look_at(vp, &eye, &focus, &T3D_VEC3_ZUP);
}

void player_free(void)
{
	inventory_free(&player.inventory);
	player.colmesh_ptrs[0] = player.colmesh_ptrs[1] = NULL;
	player.yaw = player.yaw_old = player.pitch = player.pitch_old = 0.f;
	player.pos = player.pos_old = T3D_VEC3_ZERO;
}
