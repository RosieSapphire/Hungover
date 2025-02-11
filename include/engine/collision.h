#ifndef _ENGINE_COLLISION_H_
#define _ENGINE_COLLISION_H_

#include <stdio.h>
#include <stdint.h>
#ifndef IS_USING_SCENE_CONVERTER
#include <t3d/t3d.h>
#endif

typedef struct {
	float pos[3];
} CollisionVertex;

typedef struct {
	CollisionVertex verts[3];
	float norm[3];
} CollisionTriangle;

typedef struct {
	uint16_t numTriangles;
	CollisionTriangle *triangles;
	T3DVec3 offset;
} CollisionMesh;

CollisionMesh collisionMeshInitFromFile(FILE *file, const T3DVec3 *offset);
void collisionMeshFree(CollisionMesh *cm);

#endif /* _ENGINE_COLLISION_H_ */
