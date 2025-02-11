#include <libdragon.h>

#include "engine/ui.h"

static sprite_t *buttonASpr = NULL;
static int elementFlags = 0;

void uiInit(void)
{
	buttonASpr = sprite_load("rom:/ButtonA.ci4.sprite");
	elementFlags = 0;
}

void uiToggleElements(const int flags, const int toggle)
{
	if (toggle) {
		elementFlags |= flags;
	} else {
		elementFlags &= ~(flags);
	}
}

void uiRender(void)
{
	rspq_wait();
	rdpq_set_mode_copy(true);

	if (elementFlags & UI_ELEMENT_FLAG_A_BUTTON) {
		rdpq_sprite_blit(buttonASpr, 144, 170, NULL);
	}
}

void uiFree(void)
{
	sprite_free(buttonASpr);
	buttonASpr = NULL;
	elementFlags = 0;
}
