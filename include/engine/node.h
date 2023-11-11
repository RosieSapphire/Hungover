#ifndef ENGINE_NODE_H_
#define ENGINE_NODE_H_

#include "engine/config.h"

typedef struct node_t node_t;

struct node_t
{
	char name[CONF_NAME_MAX_LEN];
	uint16_t mesh_index;
	float mat[4 * 4];
	uint16_t num_children;
	node_t *children;
	bool is_active;
};

void node_destroy(node_t *n);

#endif /* ENGINE_NODE_H_ */
