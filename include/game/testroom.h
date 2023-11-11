#ifndef GAME_TESTROOM_H
#define GAME_TESTROOM_H

#include <libdragon.h>

#include "engine/scene.h"
#include "engine/update.h"

enum scene_index testroom_update(struct update_parms uparms);
void testroom_draw(float subtick);

#endif /* GAME_TESTROOM_H */
