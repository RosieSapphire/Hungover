#include "engine/player.h"

#include "util.h"

struct player player_create(const T3DVec3 *spawn_pos, const float spawn_yaw,
                            const float spawn_pitch)
{
        struct player p;

        p.position_a = *spawn_pos;
        p.position_b = p.position_a;
        p.yaw_a = spawn_yaw;
        p.yaw_b = p.yaw_a;
        p.pitch_a = spawn_pitch;
        p.pitch_b = p.pitch_a;

        return p;
}

void player_update(struct player *p, const joypad_inputs_t *inp,
                   const float ft)
{
        p->yaw_a = p->yaw_b;
        p->yaw_b -= (inp->stick_x * .0625f) * ft;

        p->pitch_a = p->pitch_b;
        p->pitch_b -= (inp->stick_y * .0625f) * ft;
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
        float yaw, pitch;

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
