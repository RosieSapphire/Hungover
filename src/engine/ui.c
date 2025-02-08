#include <libdragon.h>

#include "engine/ui.h"

static sprite_t *button_a_spr = NULL;
static int element_flags = 0;

void ui_init(void)
{
	button_a_spr = sprite_load("rom:/ButtonA.ci4.sprite");
	element_flags = 0;
}

void ui_toggle_elements(const int flags, const int toggle)
{
	if (toggle) {
		element_flags |= flags;
	} else {
		element_flags &= ~(flags);
	}
}

void ui_render(void)
{
	rspq_wait();
	rdpq_set_mode_copy(true);

	if (element_flags & UI_ELEMENT_FLAG_A_BUTTON) {
		rdpq_sprite_blit(button_a_spr, 144, 170, NULL);
	}
}

void ui_terminate(void)
{
	sprite_free(button_a_spr);
	button_a_spr = NULL;
	element_flags = 0;
}
