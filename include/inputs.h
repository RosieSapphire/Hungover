#pragma once

#include "util.h"

enum {
        BTN_A,
        BTN_B,
        BTN_Z,
        BTN_START,
        BTN_C_UP,
        BTN_C_DOWN,
        BTN_C_LEFT,
        BTN_C_RIGHT,
        BTN_D_UP,
        BTN_D_DOWN,
        BTN_D_LEFT,
        BTN_D_RIGHT,
        BTN_COUNT
};

struct inputs {
        T3DVec2 stick;
        float stick_mag;
        bool btn[BTN_COUNT];
};

struct inputs inputs_get_from_libdragon(void);
struct inputs inputs_get_diff(const struct inputs *inp_old,
                              const struct inputs *inp_new);
