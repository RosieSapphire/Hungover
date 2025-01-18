#ifndef _ENGINE_CONTROLLER_H_
#define _ENGINE_CONTROLLER_H_

#include <libdragon.h>

enum {
	BUTTON_FLAG_DOWN_NOW_SHIFT = 0,
	BUTTON_FLAG_DOWN_LAST_SHIFT,
	BUTTON_FLAG_PRESSED_SHIFT,
	BUTTON_FLAG_RELEASED_SHIFT,
};

enum {
	BUTTON_FLAG_DOWN_NOW = (1 << 0),
	BUTTON_FLAG_DOWN_LAST = (1 << 1),
	BUTTON_FLAG_PRESSED = (1 << 2),
	BUTTON_FLAG_RELEASED = (1 << 3),
	BUTTON_FLAG_MASK = 0b1111
};

enum {
	CONTROLLER_FLAG_PORT1 = (1 << 0),
	CONTROLLER_FLAG_PORT2 = (1 << 1),
	CONTROLLER_FLAG_PORT3 = (1 << 2),
	CONTROLLER_FLAG_PORT4 = (1 << 3),
	CONTROLLER_FLAG_MASK = 0b1111
};

enum {
	BUTTON_A = 0,
	BUTTON_B,
	BUTTON_Z,
	BUTTON_START,
	BUTTON_C_LEFT,
	BUTTON_C_RIGHT,
	BUTTON_C_UP,
	BUTTON_C_DOWN,
	NUM_BUTTONS
};

#define NUM_CONTROLLER_PORTS 4

/* helper macros for getting buttons and stick (PORT is 1 - 4 [starts at 1]) */
#define BUTTON_GET_FLAG(BUTTON, FLAG, PORT)                  \
	((controllers[(PORT - 1)].buttons[BUTTON_##BUTTON] & \
	  (BUTTON_FLAG_##FLAG)) >>                           \
	 BUTTON_FLAG_##FLAG##_SHIFT)

/* `COMP` can be either X, Y, or MAG */
#define STICK_GET(COMP, PORT)                          \
	(!strncmp(#COMP, "MAG", 3) ?                   \
		 (controllers[(PORT - 1)].stick_mag) : \
		 (controllers[(PORT - 1)].stick[#COMP[0] == 'Y']))

typedef struct {
	uint8_t buttons[NUM_BUTTONS];
	float stick[2], stick_mag;
} controller_t;

extern controller_t controllers[NUM_CONTROLLER_PORTS];

void controllers_init(uint8_t controllers_in_use_flags);
void controllers_update();
void controllers_terminate(void);

#endif /* _ENGINE_CONTROLLER_H_ */
