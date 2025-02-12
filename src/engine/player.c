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

void player_to_viewport(T3DViewport *vp, const struct player *p,
			const f32 interp);
void player_free(struct player *p);

struct player player_init(void)
{
	struct player p;

	p.pos = T3D_VEC3_ZERO;
	p.pos_old = T3D_VEC3_ZERO;
	p.yaw = 0.f;
	p.yaw_old = 0.f;
	p.pitch = 0.f;
	p.pitch_old = 0.f;
	p.colmesh_ptrs[0] = NULL;
	p.colmesh_ptrs[1] = NULL;

	return p;
}

void player_look_values_get(T3DVec3 *eye, T3DVec3 *focus,
			    const struct player *p, const f32 interp)
{
	/* eye */
	T3DVec3 pos_lerp;
	T3DVec3 eye_offset;

	t3d_vec3_lerp(&pos_lerp, &p->pos_old, &p->pos, interp);
	t3d_vec3_scale(&eye_offset, &T3D_VEC3_ZUP, PLAYER_HEIGHT);
	t3d_vec3_add(eye, &pos_lerp, &eye_offset);

	/* focus */
	f32 yaw_lerp = t3d_lerp(p->yaw_old, p->yaw, interp);
	f32 pitch_lerp = t3d_lerp(p->pitch_old, p->pitch, interp);

	*focus = T3D_VEC3(cosf(T3D_DEG_TO_RAD(yaw_lerp)) *
				  cosf(T3D_DEG_TO_RAD(pitch_lerp)),
			  sinf(T3D_DEG_TO_RAD(yaw_lerp)) *
				  cosf(T3D_DEG_TO_RAD(pitch_lerp)),
			  T3D_DEG_TO_RAD(pitch_lerp));
	t3d_vec3_add(focus, focus, eye);
}

static void _player_look_angles_update(struct player *p, const f32 dt)
{
	f32 stick[2] = { INPUT_GET_STICK(X), INPUT_GET_STICK(Y) };

	/* yaw */
	p->yaw -= stick[0] * PLAYER_TURN_DEG_PER_SEC * dt;
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

static void _player_positions_update(struct player *p, const f32 dt)
{
	T3DVec3 eye, focus, lookDir;
	player_look_values_get(&eye, &focus, p, 1.f);
	t3d_vec3_diff(&lookDir, &focus, &eye);

	const s8 forwSign =
		INPUT_GET_BTN(C_UP, HELD) - INPUT_GET_BTN(C_DOWN, HELD);
	const s8 sideSign =
		INPUT_GET_BTN(C_RIGHT, HELD) - INPUT_GET_BTN(C_LEFT, HELD);
	T3DVec3 forwMove = lookDir;
	forwMove.v[2] = 0.f;
	t3d_vec3_norm(&forwMove);
	T3DVec3 sideMove, move;
	t3d_vec3_cross(&sideMove, &forwMove, &T3D_VEC3_ZUP);
	t3d_vec3_scale(&forwMove, &forwMove, forwSign);
	t3d_vec3_scale(&sideMove, &sideMove, sideSign);

	t3d_vec3_add(&move, &forwMove, &sideMove);
	t3d_vec3_norm(&move);
	t3d_vec3_scale(&move, &move, PLAYER_MOVE_UNITS_PER_SEC * dt);

	t3d_vec3_add(&p->pos, &p->pos, &move);
}

static void _player_collision_update(struct player *p, const u16 col_index)
{
	struct collision_mesh *cm = p->colmesh_ptrs[col_index];

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
		T3DVec3 checkOffset;
		T3DVec3 checkPos;

		t3d_vec3_scale(&checkOffset, &T3D_VEC3_ZUP,
			       PLAYER_COLLISION_CHECK_Y_OFFSET);
		t3d_vec3_add(&checkPos, &p->pos, &checkOffset);

		if (!t3d_raycast_triangle(&checkPos, &dir, tri_verts, &dist)) {
			continue;
		}

		if (dist > PLAYER_MAX_COLLISION_DIST) {
			continue;
		}

		T3DVec3 pushVec;
		f32 pushAmnt = 0.f;

		const f32 triDot = t3d_vec3_dot(&triNorm, &T3D_VEC3_ZUP);
		if (!triDot) { /* is wall */
			pushAmnt = fmaxf(PLAYER_RADIUS - dist, 0.f);
		} else if (triDot > 0.4f) { /* is floor */
			pushAmnt = -dist + PLAYER_COLLISION_CHECK_Y_OFFSET;
		}

		t3d_vec3_scale(&pushVec, &triNorm, pushAmnt);
		t3d_vec3_add(&p->pos, &pushVec, &p->pos);
	}
}

void player_update(struct player *p, const struct scene *scn, const f32 dt)
{
	/* old values */
	p->yaw_old = p->yaw;
	p->pitch_old = p->pitch;
	p->pos_old = p->pos;

#if PLAYER_ENABLE_COLLISION
	p->colmesh_ptrs[0] = &scn->areas[scn->area_index].colmesh;
	p->colmesh_ptrs[1] = (scn->flags & SCENE_FLAG_PROCESS_AREA_LAST) ?
				     &scn->areas[scn->area_index_old].colmesh :
				     NULL;
#else
	p->colmesh_ptrs[0] = NULL;
	p->colmesh_ptrs[1] = NULL;
#endif

	_player_look_angles_update(p, dt);
	_player_positions_update(p, dt);

	for (u8 i = 0; i < 2; i++) {
		if (p->colmesh_ptrs[i]) {
			_player_collision_update(p, i);
		}
	}

	/* TODO: Collision with objects */
}

void player_to_viewport(T3DViewport *vp, const struct player *p,
			const f32 interp)
{
	T3DVec3 eye, focus;

	player_look_values_get(&eye, &focus, p, interp);
	t3d_viewport_look_at(vp, &eye, &focus, &T3D_VEC3_ZUP);
}

void playerFree(struct player *p)
{
	p->colmesh_ptrs[0] = p->colmesh_ptrs[1] = NULL;
	p->yaw = p->yaw_old = p->pitch = p->pitch_old = 0.f;
	p->pos = p->pos_old = T3D_VEC3_ZERO;
}
