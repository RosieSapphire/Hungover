#include <stdbool.h>
#include <string.h>
#include <malloc.h>

#include "engine/types.h"
#include "engine/node.h"

/**
 * node_destroy - Destroys a Node
 * @n: Node in Question
 */
void node_destroy(struct node *n)
{
	memset(n->name, 0, CONF_NAME_MAX_LEN);
	memset(n->mat, 0, 16 * sizeof(f32));

	for (int i = 0; i < n->num_children; i++)
		node_destroy(n->children + i);

	free(n->children);
	n->mesh_index = n->num_children = n->is_active = 0;
}
