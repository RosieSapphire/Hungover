#ifndef _INPUT_H_
#define _INPUT_H_

#include <stdint.h>

#define INPUT_GET_BTN(BUTTON, STATE)                                      \
	((inputNew.buttonFlags[BUTTON_##BUTTON] & BUTTON_FLAG_##STATE) >> \
	 BUTTON_FLAG_##STATE##_SHIFT)
#define INPUT_GET_STICK(COMP)                        \
	((STICK_COMP_##COMP < STICK_COMP_MAG) ?      \
		 inputNew.stick[STICK_COMP_##COMP] : \
		 inputNew.stickMag)

#define STICK_MAG_MIN 10
#define STICK_MAG_MAX 64

enum {
	STICK_COMP_X,
	STICK_COMP_Y,
	STICK_COMP_MAG,
};

enum {
	BUTTON_A,
	BUTTON_B,
	BUTTON_Z,
	BUTTON_L,
	BUTTON_R,
	BUTTON_START,
	BUTTON_DPAD_LEFT,
	BUTTON_DPAD_RIGHT,
	BUTTON_DPAD_UP,
	BUTTON_DPAD_DOWN,
	BUTTON_C_LEFT,
	BUTTON_C_RIGHT,
	BUTTON_C_UP,
	BUTTON_C_DOWN,
	BUTTON_COUNT
};

enum {
	BUTTON_FLAG_HELD_SHIFT,
	BUTTON_FLAG_PRESSED_SHIFT,
	BUTTON_FLAG_RELEASED_SHIFT
};

enum {
	BUTTON_FLAG_HELD = (1 << BUTTON_FLAG_HELD_SHIFT),
	BUTTON_FLAG_PRESSED = (1 << BUTTON_FLAG_PRESSED_SHIFT),
	BUTTON_FLAG_RELEASED = (1 << BUTTON_FLAG_RELEASED_SHIFT)
};

typedef struct {
	uint8_t buttonFlags[BUTTON_COUNT];
	float stick[2], stickMag;
} input_t;

extern input_t inputNew, inputOld;

void inputInit(void);
void inputPoll(void);
void inputFree(void);

#endif /* _INPUT_H_ */
