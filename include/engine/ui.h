#ifndef _ENGINE_UI_H_
#define _ENGINE_UI_H_

#include "types.h"

enum {
	UI_ELEMENT_FLAG_A_BUTTON = (1 << 0),
};

void ui_init(void);
void ui_elements_toggle(const u8 flags, const boolean toggle);
void ui_render(void);
void ui_free(void);

#endif /* _ENGINE_UI_H_ */
