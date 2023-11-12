#ifndef ENGINE_NODE_H_
#define ENGINE_NODE_H_

#include "engine/config.h"

/**
 * struct node - Node Structure
 * @name: Node Name
 * @mesh_index: Scene Mesh Index
 * @mat: Transformation Matrix
 * @num_children: Number of Node Children
 * @children: Children Array
 * @is_active: Boolean for Is Active
 */
struct node
{
	char name[CONF_NAME_MAX_LEN];
	u16 mesh_index;
	f32 mat[4 * 4];
	u16 num_children;
	struct node *children;
	u8 is_active;
};

void node_destroy(struct node *n);

#endif /* ENGINE_NODE_H_ */
