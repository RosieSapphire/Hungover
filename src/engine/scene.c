#include "engine/scene.h"

Scene sceneInitFromFile(const char *path)
{
	Scene scn;
	char mdlName[32];
	char mdlPath[64];

	memset(mdlName, 0, 32);
	memset(mdlPath, 0, 64);

	strncpy(mdlName, path, strlen(path) - 4);
	snprintf(mdlPath, 64, "%s.t3dm", mdlName);
	scn.mdl = t3d_model_load(mdlPath);

	scn.areaIndex = scn.areaIndexOld = 0;
	scn.flags = 0;

	FILE *file = asset_fopen(path, NULL);
	assertf(file, "Failed to load scene from '%s'\n", path);

	fread(&scn.numAreas, 2, 1, file);
	scn.areas = calloc(scn.numAreas, sizeof *scn.areas);
	for (uint16_t i = 0; i < scn.numAreas; i++) {
		scn.areas[i] = areaInitFromFile(file, scn.mdl, i);
	}

	fclose(file);

	return scn;
}

static void _sceneUpdateAreaObject(Object *o, Scene *scn,
				   const T3DVec3 *playerPos,
				   const T3DVec3 *playerDir, uint16_t *inds[2],
				   const float dt)
{
	if (!(o->flags & OBJECT_FLAG_IS_ACTIVE)) {
		return;
	}

	switch (objectUpdate(o, playerPos, playerDir, dt)) {
	case OBJECT_UPDATE_RETURN_LOAD_NEXT_AREA:
		*inds[1] = *inds[0];
		*inds[0] = o->argi[OBJECT_DOOR_ARGI_NEXT_AREA];
		scn->flags |= SCENE_FLAG_PROCESS_AREA_LAST;
		Object *newDoor = areaFindDoorFromDestIndex(
			scn->areas + o->argi[OBJECT_DOOR_ARGI_NEXT_AREA],
			*inds[1]);
		newDoor->flags &= ~(OBJECT_FLAG_IS_ACTIVE);
		return;

	case OBJECT_UPDATE_RETURN_UNLOAD_PREV_AREA:
		scn->flags &= ~(SCENE_FLAG_PROCESS_AREA_LAST);
		Object *curDoor = areaFindDoorFromDestIndex(
			scn->areas + o->argi[OBJECT_DOOR_ARGI_NEXT_AREA],
			*inds[1]);
		curDoor->flags |= OBJECT_FLAG_IS_ACTIVE;
		return;

	case OBJECT_UPDATE_RETURN_UNLOAD_NEXT_AREA:
		scn->flags &= ~(SCENE_FLAG_PROCESS_AREA_LAST);
		scn->areaIndex = scn->areaIndexOld;
		return;

	default:
		return;
	}
}

void sceneUpdate(Scene *scn, const T3DVec3 *playerPos, const T3DVec3 *playerDir,
		 const float dt)
{
	uint16_t *inds[2] = { &scn->areaIndex, &scn->areaIndexOld };

	/*
	 * This is to ensure all objects are only updated once per frame.
	 * The reason this is done is because if, say, a door were to 
	 * load a new area, the previous area would be active as well,
	 * causing that same door object to be updated twice,
	 * causing various issues. This loop mitigates that.
	 */
	objectSetupFrameStaticVars();
	for (uint16_t i = 0; i < scn->numAreas; i++) {
		Area *a = scn->areas + i;

		for (uint16_t j = 0; j < a->numObjects; j++) {
			Object *o = a->objects + j;

			o->flags &= ~(OBJECT_FLAG_WAS_UPDATED_THIS_FRAME);
		}
	}

	for (uint16_t i = 0; i < 2; i++) {
		if (i && !(scn->flags & SCENE_FLAG_PROCESS_AREA_LAST)) {
			continue;
		}

		Area *area = scn->areas + *inds[i];
		for (uint16_t j = 0; j < area->numObjects; j++) {
			_sceneUpdateAreaObject(area->objects + j, scn,
					       playerPos, playerDir, inds, dt);
		}
	}
	objectUpdateUIWithStaticVars();
}

void sceneRender(const Scene *scn, const float subtick)
{
	areaRender(scn->areas + scn->areaIndex, subtick);
	if ((scn->flags & SCENE_FLAG_PROCESS_AREA_LAST) &&
	    scn->areaIndex ^ scn->areaIndexOld) {
		areaRender(scn->areas + scn->areaIndexOld, subtick);
	}
}

void sceneFree(Scene *scn)
{
	for (uint16_t i = 0; i < scn->numAreas; i++) {
		areaFree(scn->areas + i);
	}
	free(scn->areas);
	scn->areas = NULL;
	scn->numAreas = 0;
}
