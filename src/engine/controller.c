#include <libdragon.h>

#include "engine/controller.h"

/* DEBUG MACROS */
// #define CONTROLLER_DEBUG_BUTTONS
// #define CONTROLLER_DEBUG_STICK
#if defined CONTROLLER_DEBUG_BUTTONS || defined CONTROLLER_DEBUG_STICK
#define CONTROLLER_DEBUG
#endif

#define STICK_MAG_MAX 1.f
#define STICK_MAG_MIN .0625f

static uint8_t controller_used_flags = 0;

controller_t controllers[NUM_CONTROLLER_PORTS];

void controllers_init(uint8_t controllers_in_use_flags)
{
#ifdef CONTROLLER_DEBUG
	assertf(controllers_in_use_flags,
		"`controllers_init` ran with no usage flags"
		" (CONTROLLER_FLAG_PORT#)\n");
#endif

	joypad_init();
	controller_used_flags = controllers_in_use_flags;
	for (int i = 0; i < NUM_CONTROLLER_PORTS; i++) {
		if (!(controller_used_flags & (1 << i))) {
			continue;
		}

		controller_t *c = controllers + i;
		memset(c->buttons, 0, NUM_BUTTONS);
		for (int j = 0; j < 2; j++) {
			c->stick[j] = 0.f;
		}
		c->stick_mag = 0.0f;
#ifdef CONTROLLER_DEBUG
		debugf("Initialized controller %d\n", i + 1);
#endif
	}
}

void controllers_update(void)
{
	joypad_poll();

	for (int i = 0; i < NUM_CONTROLLER_PORTS; i++) {
		if (!(controller_used_flags & (1 << i))) {
			continue;
		}

		/* BUTTONS */
		joypad_inputs_t jp = joypad_get_inputs(i);
		controller_t *c = controllers + i;
		uint8_t jp_btn[NUM_BUTTONS] = {
			jp.btn.a,     jp.btn.b,	     jp.btn.z,
			jp.btn.start, jp.btn.c_left, jp.btn.c_right,
			jp.btn.c_up,  jp.btn.c_down,
		};

		for (int j = 0; j < NUM_BUTTONS; j++) {
			/* last input */
			c->buttons[j] &= BUTTON_FLAG_DOWN_NOW;
			c->buttons[j] =
				BUTTON_FLAG_DOWN_LAST *
				((c->buttons[j] & BUTTON_FLAG_DOWN_NOW) >>
				 BUTTON_FLAG_DOWN_NOW_SHIFT);

			/* new input */
			c->buttons[j] &= ~(BUTTON_FLAG_DOWN_NOW);
			c->buttons[j] |= jp_btn[j]
					 << BUTTON_FLAG_DOWN_NOW_SHIFT;

			const int now = (c->buttons[j] & BUTTON_FLAG_DOWN_NOW) >
					0;
			const int last =
				(c->buttons[j] & BUTTON_FLAG_DOWN_LAST) >>
				BUTTON_FLAG_DOWN_LAST_SHIFT;

			/* was just pressed or released */
			c->buttons[j] |= ((now && (now ^ last))
					  << BUTTON_FLAG_PRESSED_SHIFT) |
					 ((!now && (now ^ last))
					  << BUTTON_FLAG_RELEASED_SHIFT);
		}

#ifdef CONTROLLER_DEBUG_BUTTONS
		debugf("Port %d -> Z - Now: %d, Last: %d, "
		       "Press: %d, Release: %d\n",
		       i + 1,
		       ((c->buttons[BUTTON_Z] & BUTTON_FLAG_DOWN_NOW) >>
			BUTTON_FLAG_DOWN_NOW_SHIFT),
		       ((c->buttons[BUTTON_Z] & BUTTON_FLAG_DOWN_LAST) >>
			BUTTON_FLAG_DOWN_LAST_SHIFT),
		       ((c->buttons[BUTTON_Z] & BUTTON_FLAG_PRESSED) >>
			BUTTON_FLAG_PRESSED_SHIFT),
		       ((c->buttons[BUTTON_Z] & BUTTON_FLAG_RELEASED) >>
			BUTTON_FLAG_RELEASED_SHIFT));
#endif
		/* STICK */
		c->stick[0] = (float)jp.stick_x / 64.f;
		c->stick[1] = (float)jp.stick_y / 64.f;
		c->stick_mag = sqrtf(c->stick[0] * c->stick[0] +
				     c->stick[1] * c->stick[1]);
		if (c->stick_mag < STICK_MAG_MIN) {
			c->stick_mag = c->stick[0] = c->stick[1] = .0f;
		}
		if (c->stick_mag > STICK_MAG_MAX) {
			const float mul = STICK_MAG_MAX / c->stick_mag;
			c->stick[0] *= mul;
			c->stick[1] *= mul;
			c->stick_mag = STICK_MAG_MAX;
		}
#ifdef CONTROLLER_DEBUG_STICK
		debugf("Port %d Stick -> Vec: (%f, %f) Mag: %f\n", i,
		       c->stick[0], c->stick[1], c->stick_mag);
#endif
	}
#ifdef CONTROLLER_DEBUG_BUTTONS
	debugf("\n");
#endif
}

void controllers_terminate(void)
{
	joypad_close();
	for (int i = 0; i < NUM_CONTROLLER_PORTS; i++) {
		controller_t *c = controllers + i;
		memset(c->buttons, 0, NUM_BUTTONS);
		for (int j = 0; j < 2; j++) {
			c->stick[j] = 0.f;
		}
		c->stick_mag = 0.0f;
	}
}
