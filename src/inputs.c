#include "inputs.h"

#include "util.h"

static float inputs_get_stick_from_libdragon(T3DVec2 *stick_out,
                                             const int8_t stick_in_x,
                                             const int8_t stick_in_y)
{
        T3DVec2 v;
        float mag;

        v.v[0] = (float)stick_in_x / JOYSTICK_MAG_MAX;
        v.v[1] = (float)stick_in_y / JOYSTICK_MAG_MAX;
        mag = sqrtf(v.v[0] * v.v[0] + v.v[1] * v.v[1]);
        if (!mag) {
                *stick_out = t3d_vec2_zero();
                return 0.f;
        }

        if (mag < ((float)JOYSTICK_MAG_MIN / (float)JOYSTICK_MAG_MAX)) {
                *stick_out = t3d_vec2_zero();
                return 0.f;
        }

        if (mag >= 1.f) {
                v.v[0] /= mag;
                v.v[1] /= mag;
                mag = 1.f;
        }

        *stick_out = v;

        return mag;
}

struct inputs inputs_get_from_libdragon(void)
{
        joypad_inputs_t inp_src;
        struct inputs inp_dst;

        joypad_poll();
        inp_src = joypad_get_inputs(JOYPAD_PORT_1);
        inp_dst.stick_mag = inputs_get_stick_from_libdragon(&inp_dst.stick,
                                                            inp_src.stick_x,
                                                            inp_src.stick_y);

        inp_dst.btn[BTN_A] = inp_src.btn.a;
        inp_dst.btn[BTN_B] = inp_src.btn.b;
        inp_dst.btn[BTN_Z] = inp_src.btn.z;
        inp_dst.btn[BTN_START] = inp_src.btn.start;
        inp_dst.btn[BTN_C_UP] = inp_src.btn.c_up;
        inp_dst.btn[BTN_C_DOWN] = inp_src.btn.c_down;
        inp_dst.btn[BTN_C_LEFT] = inp_src.btn.c_left;
        inp_dst.btn[BTN_C_RIGHT] = inp_src.btn.c_right;
        inp_dst.btn[BTN_D_UP] = inp_src.btn.d_up;
        inp_dst.btn[BTN_D_DOWN] = inp_src.btn.d_down;
        inp_dst.btn[BTN_D_LEFT] = inp_src.btn.d_left;
        inp_dst.btn[BTN_D_RIGHT] = inp_src.btn.d_right;

        return inp_dst;
}

struct inputs inputs_get_diff(const struct inputs *inp_old,
                              const struct inputs *inp_new)
{
        struct inputs inp_diff;
        int i;

        inp_diff.stick.v[0] = inp_new->stick.v[0] - inp_old->stick.v[0];
        inp_diff.stick.v[1] = inp_new->stick.v[1] - inp_old->stick.v[1];

        for (i = 0; i < BTN_COUNT; ++i)
                inp_diff.btn[i] = (inp_new->btn[i] ^ inp_old->btn[i]);


        return inp_diff;
}
