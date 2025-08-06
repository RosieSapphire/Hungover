#pragma once

#include <t3d/t3d.h>

#include "util.h"
#include "inputs.h"

#define PLAYER_HEIGHT 1.25f

struct player {
        T3DVec3 position_a;
        T3DVec3 position_b;
        T3DVec3 velocity;
        float yaw_tar;
        float yaw_a;
        float yaw_b;
        float pitch_tar;
        float pitch_a;
        float pitch_b;
};

struct player player_create(const T3DVec3 *spawn_pos, const float spawn_yaw,
                            const float spawn_pitch);
T3DVec3 player_get_forward_dir(const struct player *p, const float subtick);
T3DVec3 player_get_right_dir(const T3DVec3 *forw_dir);
void player_update(struct player *p, const struct inputs *inp, const float ft);
void player_to_view_matrix(const struct player *p, T3DViewport *vp,
                           const float subtick);
