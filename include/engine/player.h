#ifndef _ENGINE_PLAYER_H_
#define _ENGINE_PLAYER_H_

#include <t3d/t3d.h>

#include "config.h"

#include "engine/collision.h"
#include "engine/scene.h"

#define PLAYER_NUM_COLLISION_MESH_PTRS 2

typedef struct {
	T3DVec3 pos, posOld;
	float yaw, yawOld, pitch, pitchOld;
	CollisionMesh *collisionMeshPtrs[PLAYER_NUM_COLLISION_MESH_PTRS];
} Player;

Player playerInit(void);
void playerGetLookValues(T3DVec3 *eye, T3DVec3 *focus, const Player *p,
			 const float interp);
void playerUpdate(Player *p, const Scene *scn, const float dt);
void playerToViewport(T3DViewport *vp, const Player *p, const float interp);
void playerFree(Player *p);

#endif /* _ENGINE_PLAYER_H_ */
