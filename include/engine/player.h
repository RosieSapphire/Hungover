#pragma once

#include <t3d/t3d.h>

#include "util.h"
#include "inputs.h"

#define PLAYER_HEIGHT 1.25f

enum {
        PLAYER_STATE_STANDING,
        PLAYER_STATE_MOVING,
        PLAYER_STATE_COUNT
};

struct player {
        T3DVec3 position_a;
        T3DVec3 position_b;
        T3DVec3 up_real;
        T3DVec3 up_visual_a;
        T3DVec3 up_visual_b;
        T3DVec3 velocity;
        float head_wiggle_amount;
        float yaw_tar;
        float yaw_a;
        float yaw_b;
        float yaw_offset_a;
        float yaw_offset_b;
        float pitch_tar;
        float pitch_a;
        float pitch_b;
        float pitch_offset_a;
        float pitch_offset_b;
        float state_timer;
        uint8_t state;
};

struct player player_create(const T3DVec3 *spawn_pos, const float spawn_yaw,
                            const float spawn_pitch);
T3DVec3 player_get_forward_dir(const struct player *p, const float subtick,
                               bool get_offset);
T3DVec3 player_get_right_dir(const struct player *p, const T3DVec3 *forw_dir);
void player_update(struct player *p, const struct inputs *inp, const float ft);
void player_to_view_matrix(const struct player *p, T3DViewport *vp,
                           const float subtick);
