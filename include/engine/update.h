#ifndef _ENGINE_UPDATE_H_
#define _ENGINE_UPDATE_H_

#include <libdragon.h>

/**
 * struct update_parms - Update/Input Parameters
 * @down: Buttons Just Pressed
 * @held: Buttons Held Down
 * @stick: Stick Input
 */
struct update_parms
{
	joypad_buttons_t down, held;
	joypad_inputs_t stick;
};

#endif /* _ENGINE_UPDATE_H_ */
