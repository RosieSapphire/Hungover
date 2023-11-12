#include <GL/gl.h>

#include "engine/util.h"
#include "engine/scene.h"

static void _scene_node_draw(const scene_t *s, const node_t *n, f32 subtick)
{
	if (!n->is_active)
		return;

	if (n->mesh_index == 0xFFFF)
	{
		for (int i = 0; i < n->num_children; i++)
			_scene_node_draw(s, n->children + i, subtick);

		return;
	}

	u16 anim_index = 0xFFFF;

	for (u16 j = 0; j < s->num_anims; j++)
	{
		if (s->anims[j].mesh_index == n->mesh_index)
		{
			anim_index = j;
			break;
		}
	}
	glPushMatrix();
	if (anim_index != 0xFFFF)
		animation_setup_matrix(s->anims + anim_index, subtick);
	else
		glMultMatrixf(n->mat);

	const f32 pickup_spin_lerp =
		lerpf(pickup_spin_frame_last, pickup_spin_frame, subtick);
	const smesh_t *mesh = s->meshes + n->mesh_index;

	if (!strncmp(mesh->name, "PU.", 3))
	{
		glRotatef(pickup_spin_lerp * 8, 0, 0, 1);
		glTranslatef(0, 0, sinf(pickup_spin_lerp * 0.125f) * 0.25f);
	}
	smesh_draw(s, mesh);
	for (int i = 0; i < n->num_children; i++)
		_scene_node_draw(s, n->children + i, subtick);
	glPopMatrix();
}

void scene_draw(const scene_t *s, f32 subtick)
{
	_scene_node_draw(s, &s->root_node, subtick);
}

node_t *scene_node_from_name(node_t *n, const char *name)
{
	if (strcmp(n->name, name) == 0)
		return (n);

	node_t *f;

	for (int i = 0; i < n->num_children; i++)
	{
		f = scene_node_from_name(n->children + i, name);

		if (f)
			return (f);
	}

	return (NULL);
}
