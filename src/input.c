#include <libdragon.h>

#include "input.h"

input_t inputNew, inputOld;

void inputInit(void)
{
	joypad_init();
	for (int i = 0; i < BUTTON_COUNT; i++) {
		inputNew.buttonFlags[i] = 0;
		inputOld.buttonFlags[i] = 0;
	}
	inputNew.stick[0] = inputNew.stick[1] = inputOld.stick[0] =
		inputOld.stick[1] = 0.f;
}

void inputPoll(void)
{
	joypad_inputs_t jp;

	joypad_poll();
	jp = joypad_get_inputs(JOYPAD_PORT_1);

	/* buttons */
	const uint8_t jp_vals[BUTTON_COUNT] = {
		jp.btn.a,    jp.btn.b,	    jp.btn.z,	   jp.btn.l,
		jp.btn.r,    jp.btn.start,  jp.btn.d_left, jp.btn.d_right,
		jp.btn.d_up, jp.btn.d_down, jp.btn.c_left, jp.btn.c_right,
		jp.btn.c_up, jp.btn.c_down,
	};

	inputOld = inputNew;
	for (int i = 0; i < BUTTON_COUNT; i++) {
		inputNew.buttonFlags[i] = 0;
		inputNew.buttonFlags[i] |= jp_vals[i] << BUTTON_FLAG_HELD_SHIFT;
		inputNew.buttonFlags[i] |=
			(((inputNew.buttonFlags[i] & BUTTON_FLAG_HELD) >>
			  BUTTON_FLAG_HELD_SHIFT) &&
			 (((inputOld.buttonFlags[i] & BUTTON_FLAG_HELD) >>
			   BUTTON_FLAG_HELD_SHIFT) ^
			  ((inputNew.buttonFlags[i] & BUTTON_FLAG_HELD) >>
			   BUTTON_FLAG_HELD_SHIFT)))
			<< BUTTON_FLAG_PRESSED_SHIFT;
		inputNew.buttonFlags[i] |=
			(((inputOld.buttonFlags[i] & BUTTON_FLAG_HELD) >>
			  BUTTON_FLAG_HELD_SHIFT) &&
			 (((inputOld.buttonFlags[i] & BUTTON_FLAG_HELD) >>
			   BUTTON_FLAG_HELD_SHIFT) ^
			  ((inputNew.buttonFlags[i] & BUTTON_FLAG_HELD) >>
			   BUTTON_FLAG_HELD_SHIFT)))
			<< BUTTON_FLAG_RELEASED_SHIFT;
	}

	/* stick */
	inputNew.stick[0] = (float)jp.stick_x;
	inputNew.stick[1] = (float)jp.stick_y;
	inputNew.stickMag = sqrtf(inputNew.stick[0] * inputNew.stick[0] +
				  inputNew.stick[1] * inputNew.stick[1]);

	/* normalize the stick so it can't go farther than 64 units */
	if (inputNew.stickMag < STICK_MAG_MIN) {
		inputNew.stickMag = 0.f;
		inputNew.stick[0] = inputNew.stick[1] = 0.f;
	}
	if (inputNew.stickMag > STICK_MAG_MAX) {
		const float mul = (float)STICK_MAG_MAX / inputNew.stickMag;
		inputNew.stick[0] *= mul;
		inputNew.stick[1] *= mul;
		inputNew.stickMag = STICK_MAG_MAX;
	}

	inputNew.stick[0] /= STICK_MAG_MAX;
	inputNew.stick[1] /= STICK_MAG_MAX;
	inputNew.stickMag /= STICK_MAG_MAX;
}

void inputFree(void)
{
	for (int i = 0; i < BUTTON_COUNT; i++) {
		inputNew.buttonFlags[i] = 0;
		inputOld.buttonFlags[i] = 0;
	}
	inputNew.stick[0] = inputNew.stick[1] = inputOld.stick[0] =
		inputOld.stick[1] = 0.f;
	joypad_close();
}
