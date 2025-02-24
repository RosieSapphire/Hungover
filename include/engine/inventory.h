#ifndef _ENGINE_INVENTORY_H_
#define _ENGINE_INVENTORY_H_

#include "types.h"

#define INV_ENT_MAX_COUNT 32

enum { INV_ENT_TYPE_SHOTGUN, INV_ENT_TYPE_COUNT };

struct inventory_entry {
	u8 type;
};

struct inventory {
	u8 entry_count;
	struct inventory_entry entries[INV_ENT_MAX_COUNT];
};

struct inventory inventory_init(void);
void inventory_free(struct inventory *inv);

#endif /* _ENGINE_INVENTORY_H_ */
