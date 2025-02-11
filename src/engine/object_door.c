#include "input.h"
#include "t3d_ext.h"

#include "engine/object_door.h"

#define DOOR_OBJECT_TURN_DEG_PER_SEC 179.f
#define DOOR_OBJECT_CHECK_RADIUS 100.f

int numDoorObjects = 0;
DoorObject doorObjects[DOOR_OBJECTS_MAX];
int numDoorsInRangeOf = 0;

int doorObjectInit(const int nextArea, const int areaIndex)
{
	int ind = numDoorObjects++;
	DoorObject *door = doorObjects + ind;

	door->areaIndex = areaIndex;
	door->nextArea = nextArea;
	door->isOpening = false;
	door->sideEntered = -1;
	door->swingAmount = door->swingAmountOld = 0.f;

	return ind;
}

DoorObject *doorObjectFindByNextAreaInArea(const uint16_t nextArea,
					   const uint16_t areaIndex)
{
	for (uint16_t i = 0; i < numDoorObjects; i++) {
		DoorObject *d = doorObjects + i;

		if (d->areaIndex != areaIndex) {
			continue;
		}

		if (d->nextArea != nextArea) {
			continue;
		}

		return d;
	}

	return NULL;
}

static void _doorObjectUpdateInsideRange(DoorObject *door,
					 const float facing_dot)
{
	if (facing_dot < 0.6f) {
		return;
	}

	numDoorsInRangeOf++;
	if (INPUT_GET_BTN(A, PRESSED)) {
		door->isOpening ^= door->swingAmount == 0.f ||
				   door->swingAmount == 90.f;
	}
}

static void _doorObjectUpdateOutsideRange(DoorObject *door)
{
	door->isOpening = false;
}

int doorObjectUpdate(DoorObject *door, const T3DVec3 *player_to_objDir,
		     const float dist_from_player,
		     const float player_facing_dot, const float dt)
{
	door->swingAmountOld = door->swingAmount;

	if (dist_from_player < DOOR_OBJECT_CHECK_RADIUS) {
		_doorObjectUpdateInsideRange(door, player_facing_dot);
	} else {
		_doorObjectUpdateOutsideRange(door);
	}

	if (door->isOpening) {
		door->swingAmount += dt * DOOR_OBJECT_TURN_DEG_PER_SEC;
		if (door->swingAmount > 90.f) {
			door->swingAmount = 90.f;
		}
	} else {
		door->swingAmount -= dt * DOOR_OBJECT_TURN_DEG_PER_SEC;
		if (door->swingAmount < 0.f) {
			door->swingAmount = 0.f;
		}
	}

	T3DVec3 obj_to_playerDir;

	t3d_vec3_negate(&obj_to_playerDir, player_to_objDir);
	const float pass_door_dot =
		t3d_vec3_dot(&obj_to_playerDir, &T3D_VEC3_XUP);

	/* door just opened */
	if (door->swingAmount > 0.f && door->swingAmountOld <= 0.f) {
		door->sideEntered = pass_door_dot >= 0.f;
		debugf("Loading next area from door %d\n", door - doorObjects);
		return OBJECT_UPDATE_RETURN_LOAD_NEXT_AREA;
	}

	/* door just closed */
	if (door->swingAmount <= 0.f && door->swingAmountOld > 0.f) {
		int sideOld = door->sideEntered;
		door->sideEntered = pass_door_dot >= 0.f;

		/* we have moved to the other side of the door */
		if (sideOld ^ door->sideEntered) {
			return OBJECT_UPDATE_RETURN_UNLOAD_PREV_AREA;
		}

		/* ... we have not */
		return OBJECT_UPDATE_RETURN_UNLOAD_NEXT_AREA;
	}

	return OBJECT_UPDATE_RETURN_NONE;
}

void doorObjectFree(DoorObject *door)
{
	door->swingAmount = door->swingAmountOld = 0.f;
	door->sideEntered = -1;
	door->isOpening = false;
	door->nextArea = -1;
	door->areaIndex = -1;
	numDoorObjects--;
}
