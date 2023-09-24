#ifndef GAME_TESTROOM_H
#define GAME_TESTROOM_H

#include <libdragon.h>

#include "engine/scene.h"
#include "engine/update.h"

enum scene_index testroom_update(update_parms_t uparms);
void testroom_draw(float subtick);

#endif /* GAME_TESTROOM_H */
