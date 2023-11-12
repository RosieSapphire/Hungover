#include <malloc.h>

#include "util.h"
#include "node.h"

/**
 * node_read - Reads node from a File
 * @n: Starting Node
 * @file: File to Write to
 * @depth: Depth into the Tree
 */
void node_read(struct node *n, FILE *file, int depth)
{
	fread(n->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	fread(&n->mesh_index, sizeof(u16), 1, file);
	n->mesh_index = u16_endian_flip(n->mesh_index);
	fread(n->mat, sizeof(f32), 4 * 4, file);
	for (int i = 0; i < 16; i++)
		n->mat[i] = f32_endian_flip(n->mat[i]);

	fread(&n->num_children, sizeof(u16), 1, file);
	n->num_children = u16_endian_flip(n->num_children);
	for (int i = 0; i < depth; i++)
		printf("\t");

	printf("name=%s, mesh_index=%d num_children=%d\n",
			n->name, n->mesh_index, n->num_children);
	matrix_print(n->mat, depth);

	n->children = malloc(sizeof(struct node) * n->num_children);
	for (int i = 0; i < n->num_children; i++)
		node_read(n->children + i, file, depth + 1);
}


/**
 * node_write - Writes node to a File
 * @n: Node in Question
 * @file: File to Write to
 */
void node_write(const struct node *n, FILE *file)
{
	fwrite(n->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	u16 mesh_index_flip = u16_endian_flip(n->mesh_index);

	fwrite(&mesh_index_flip, sizeof(u16), 1, file);
	f32 mat_flip[4 * 4];

	for (int i = 0; i < 16; i++)
		mat_flip[i] = f32_endian_flip(n->mat[i]);

	fwrite(mat_flip, sizeof(f32), 4 * 4, file);
	u16 num_children_flip = u16_endian_flip(n->num_children);

	fwrite(&num_children_flip, sizeof(u16), 1, file);
	for (int i = 0; i < n->num_children; i++)
		node_write(n->children + i, file);
}
