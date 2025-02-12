#include <libdragon.h>

#include "engine/ui.h"

static sprite_t *spr_button_a = NULL;
static u8 element_flags = 0;

void ui_init(void)
{
	spr_button_a = sprite_load("rom:/Tex.ButtonAOld.ci4.sprite");
	element_flags = 0;
}

void ui_elements_toggle(const u8 flags, const boolean toggle)
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
		rdpq_sprite_blit(spr_button_a, 144, 170, NULL);
	}
}

void ui_free(void)
{
	sprite_free(spr_button_a);
	spr_button_a = NULL;
	element_flags = 0;
}
