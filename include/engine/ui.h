#ifndef _ENGINE_UI_H_
#define _ENGINE_UI_H_

enum {
	UI_ELEMENT_FLAG_A_BUTTON = (1 << 0),
};

void uiInit(void);
void uiToggleElements(const int flags, const int toggle);
void uiRender(void);
void uiFree(void);

#endif /* _ENGINE_UI_H_ */
