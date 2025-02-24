#include "engine/inventory.h"

struct inventory inventory_init(void)
{
	struct inventory inv;
	inv.entry_count = 0;
	return inv;
}

void inventory_free(struct inventory *inv)
{
	for (u8 i = 0; i < INV_ENT_MAX_COUNT; i++) {
		inv->entries[i].type = -1;
	}
	inv->entry_count = 0;
}
