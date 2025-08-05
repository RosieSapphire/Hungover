#include "engine/player.h"

#include "util.h"

#define PLAYER_TURN_SPEED_SLOW 3.f
#define PLAYER_TURN_SPEED_FAST 16.f
#define PLAYER_TURN_LERP_SPEED 8.f

struct player player_create(const T3DVec3 *spawn_pos, const float spawn_yaw,
                            const float spawn_pitch)
{
        struct player p;

        p.position_a = *spawn_pos;
        p.position_b = p.position_a;
        p.yaw_tar = spawn_yaw;
        p.yaw_a = p.yaw_tar;
        p.yaw_b = p.yaw_tar;
        p.pitch_tar = spawn_pitch;
        p.pitch_a = p.pitch_tar;
        p.pitch_b = p.pitch_tar;

        return p;
}

static void player_update_turning(struct player *p, const struct inputs *inp,
                                  const float ft)
{
        float turn_speed, turn_lerp_t, pitch_limit;

        turn_speed = (inp->btn[BTN_Z]) ? PLAYER_TURN_SPEED_FAST :
                                         PLAYER_TURN_SPEED_SLOW;
        p->yaw_tar -= inp->stick.v[0] * turn_speed * ft;
        p->pitch_tar -= inp->stick.v[1] * turn_speed * ft;

        pitch_limit = T3D_DEG_TO_RAD(85.f);
        if (p->pitch_tar > pitch_limit)
                p->pitch_tar = pitch_limit;

        if (p->pitch_tar < -pitch_limit)
                p->pitch_tar = -pitch_limit;

        turn_lerp_t = clampf(ft * PLAYER_TURN_LERP_SPEED, 0.f, 1.f);
        p->yaw_a = p->yaw_b;
        p->yaw_b = lerpf(p->yaw_b, p->yaw_tar, turn_lerp_t);
        p->pitch_a = p->pitch_b;
        p->pitch_b = lerpf(p->pitch_b, p->pitch_tar, turn_lerp_t);
}

void player_update(struct player *p, const struct inputs *inp, const float ft)
{
        player_update_turning(p, inp, ft);
}

T3DVec3 player_get_forward_dir(const struct player *p, const float subtick)
{
        float yaw, pitch, cos_pitch;
        T3DVec3 dir;

        yaw = lerpf(p->yaw_a, p->yaw_b, subtick);
        pitch = lerpf(p->pitch_a, p->pitch_b, subtick);
        cos_pitch = cosf(pitch);
        dir = t3d_vec3_make(cosf(yaw) * cos_pitch,
                            sinf(yaw) * cos_pitch, sinf(pitch));

        return dir;
}

void player_to_view_matrix(const struct player *p, T3DViewport *vp,
                           const float subtick)
{
        T3DVec3 head_offset, eye, forw_dir, focus, up;

        /* Eye. */
        head_offset = t3d_vec3_make(0.f, 0.f, PLAYER_HEIGHT);
        t3d_vec3_lerp(&eye, &p->position_a, &p->position_b, subtick);
        t3d_vec3_add(&eye, &eye, &head_offset);
        eye = t3d_vec3_scale(&eye, MODEL_SCALE);

        /* Focus. */
        forw_dir = player_get_forward_dir(p, subtick);
        t3d_vec3_add(&focus, &eye, &forw_dir);

        up = t3d_vec3_make(0.f, 0.f, 1.f);

        t3d_viewport_look_at(vp, &eye, &focus, &up);
}
