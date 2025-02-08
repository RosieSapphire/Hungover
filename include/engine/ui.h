#ifndef _ENGINE_UI_H_
#define _ENGINE_UI_H_

enum {
	UI_ELEMENT_FLAG_A_BUTTON = (1 << 0),
};

void ui_init(void);
void ui_toggle_elements(const int flags, const int toggle);
void ui_render(void);
void ui_terminate(void);

#endif /* _ENGINE_UI_H_ */
