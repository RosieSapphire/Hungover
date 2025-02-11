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

#define PLAYER_ENABLE_COLLISION 1

Player playerInit(void)
{
	Player p;

	p.pos = p.posOld = T3D_VEC3_ZERO;
	p.yaw = p.yawOld = p.pitch = p.pitchOld = 0.f;
	p.collisionMeshPtrs[0] = p.collisionMeshPtrs[1] = NULL;

	return p;
}

void playerGetLookValues(T3DVec3 *eye, T3DVec3 *focus, const Player *p,
			 const float interp)
{
	/* eye */
	T3DVec3 posLerp;
	T3DVec3 eyeOffset;

	t3d_vec3_lerp(&posLerp, &p->posOld, &p->pos, interp);
	t3d_vec3_scale(&eyeOffset, &T3D_VEC3_ZUP, PLAYER_HEIGHT);
	t3d_vec3_add(eye, &posLerp, &eyeOffset);

	/* focus */
	float yawLerp = t3d_lerp(p->yawOld, p->yaw, interp);
	float pitchLerp = t3d_lerp(p->pitchOld, p->pitch, interp);

	*focus = T3D_VEC3(
		cosf(T3D_DEG_TO_RAD(yawLerp)) * cosf(T3D_DEG_TO_RAD(pitchLerp)),
		sinf(T3D_DEG_TO_RAD(yawLerp)) * cosf(T3D_DEG_TO_RAD(pitchLerp)),
		T3D_DEG_TO_RAD(pitchLerp));
	t3d_vec3_add(focus, focus, eye);
}

static void _playerUpdateLookAngles(Player *p, const float dt)
{
	float stick[2] = { INPUT_GET_STICK(X), INPUT_GET_STICK(Y) };

	/* yaw */
	p->yaw -= stick[0] * PLAYER_TURN_DEG_PER_SEC * dt;
	while (p->yaw >= 360.f) {
		p->yaw -= 360.f;
		p->yawOld -= 360.f;
	}
	while (p->yaw <= 0.f) {
		p->yaw += 360.f;
		p->yawOld += 360.f;
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

static void _playerUpdatePositions(Player *p, const float dt)
{
	T3DVec3 eye, focus, lookDir;
	playerGetLookValues(&eye, &focus, p, 1.f);
	t3d_vec3_diff(&lookDir, &focus, &eye);

	const int forwSign =
		INPUT_GET_BTN(C_UP, HELD) - INPUT_GET_BTN(C_DOWN, HELD);
	const int sideSign =
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

static void _playerUpdateCollision(Player *p, const uint16_t colIndex)
{
	CollisionMesh *cm = p->collisionMeshPtrs[colIndex];

	for (uint16_t i = 0; i < cm->numTriangles; i++) {
		CollisionTriangle *tri = cm->triangles + i;
		T3DVec3 dir;
		T3DVec3 triVerts[3];
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				triVerts[j].v[k] =
					tri->verts[j].pos[k] + cm->offset.v[k];
			}
		}
		T3DVec3 triNorm = *(T3DVec3 *)tri->norm;

		t3d_vec3_negate(&dir, &triNorm);

		float dist;
		T3DVec3 checkOffset;
		T3DVec3 checkPos;

		t3d_vec3_scale(&checkOffset, &T3D_VEC3_ZUP,
			       PLAYER_COLLISION_CHECK_Y_OFFSET);
		t3d_vec3_add(&checkPos, &p->pos, &checkOffset);

		if (!t3d_raycast_triangle(&checkPos, &dir, triVerts, &dist)) {
			continue;
		}

		if (dist > PLAYER_MAX_COLLISION_DIST) {
			continue;
		}

		T3DVec3 pushVec;
		float pushAmnt = 0.f;

		const float triDot = t3d_vec3_dot(&triNorm, &T3D_VEC3_ZUP);
		if (!triDot) { /* is wall */
			pushAmnt = fmaxf(PLAYER_RADIUS - dist, 0.f);
		} else if (triDot > 0.4f) { /* is floor */
			pushAmnt = -dist + PLAYER_COLLISION_CHECK_Y_OFFSET;
		}

		t3d_vec3_scale(&pushVec, &triNorm, pushAmnt);
		t3d_vec3_add(&p->pos, &pushVec, &p->pos);
	}
}

void playerUpdate(Player *p, const Scene *scn, const float dt)
{
	/* old values */
	p->yawOld = p->yaw;
	p->pitchOld = p->pitch;
	p->posOld = p->pos;

#if PLAYER_ENABLE_COLLISION
	p->collisionMeshPtrs[0] = &scn->areas[scn->areaIndex].colmesh;
	p->collisionMeshPtrs[1] =
		(scn->flags & SCENE_FLAG_PROCESS_AREA_LAST) ?
			&scn->areas[scn->areaIndexOld].colmesh :
			NULL;
#else
	p->collisionMeshPtrs[0] = p->collisionMeshPtrs[1] = NULL;
#endif

	_playerUpdateLookAngles(p, dt);
	_playerUpdatePositions(p, dt);

	/* collision with static geometry */
	for (int i = 0; i < 2; i++) {
		if (p->collisionMeshPtrs[i]) {
			_playerUpdateCollision(p, i);
		}
	}

	/* collision with objects */
}

void playerToViewport(T3DViewport *vp, const Player *p, const float interp)
{
	T3DVec3 eye, focus;

	playerGetLookValues(&eye, &focus, p, interp);
	t3d_viewport_look_at(vp, &eye, &focus, &T3D_VEC3_ZUP);
}

void playerFree(Player *p)
{
	p->collisionMeshPtrs[0] = p->collisionMeshPtrs[1] = NULL;
	p->yaw = p->yawOld = p->pitch = p->pitchOld = 0.f;
	p->pos = p->posOld = T3D_VEC3_ZERO;
}
