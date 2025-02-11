#ifndef _ENGINE_OBJECT_DOOR_H_
#define _ENGINE_OBJECT_DOOR_H_

#include "engine/object.h"

#define DOOR_OBJECTS_MAX 16

typedef struct {
	int areaIndex, nextArea, isOpening, sideEntered;
	float swingAmount, swingAmountOld;
} DoorObject;

extern int numDoorObjects;
extern DoorObject doorObjects[DOOR_OBJECTS_MAX];
extern int numDoorsInRangeOf;

int doorObjectInit(const int nextArea, const int areaIndex);
DoorObject *doorObjectFindByNextAreaInArea(const uint16_t nextArea,
					   const uint16_t areaIndex);
int doorObjectUpdate(DoorObject *door, const T3DVec3 *player_to_objDir,
		     const float dist_from_player,
		     const float player_facing_dot, const float dt);
void doorObjectFree(DoorObject *door);

#endif /* _ENGINE_OBJECT_DOOR_H_ */
