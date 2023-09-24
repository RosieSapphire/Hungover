#ifndef GAME_TITLE_H
#define GAME_TITLE_H

#include <libdragon.h>

#include "engine/scene.h"
#include "engine/update.h"

enum scene_index title_update(update_parms_t uparms);
void title_draw(float subtick);

#endif /* GAME_TITLE_H */
