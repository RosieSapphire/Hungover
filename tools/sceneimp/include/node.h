#ifndef _NODE_H_
#define _NODE_H_

#include <stdio.h>
#include "../../../include/engine/types.h"
#include "../../../include/engine/config.h"

/**
 * struct node - Node Struct
 * @name: Node Name
 * @mesh_index: Mesh Index
 * @mat: Transformation Matrix
 * @num_children: Number of Children
 * @children: Children Array
 */
struct node
{
	char name[CONF_NAME_MAX_LEN];
	u16 mesh_index;
	f32 mat[4 * 4];
	u16 num_children;
	struct node *children;
};

void node_read(struct node *n, FILE *file, int depth);
void node_write(const struct node *n, FILE *file);

#endif /* _NODE_H_ */
