#include "endian.h"

#include "export.h"

void scene_export_to_file(const struct scene *scn, FILE *file)
{
	u16 i;

	fwrite_ef16(&scn->area_count, file);
	for (i = 0; i < scn->area_count; i++) {
		area_export_to_file(scn->areas + i, file);
	}
#ifdef GLB_TO_SCN_DEBUG
	printf("Successfully wrote scene to file\n");
#endif /* GLB_TO_SCN_DEBUG */
}

void area_export_to_file(const struct area *a, FILE *file)
{
	u16 i;

	for (i = 0; i < 3; i++) {
		fwrite_ef32(a->offset.v + i, file);
	}
	collision_mesh_export_to_file(&a->colmesh, file);
	fwrite_ef16(&a->actor_header_count, file);
	for (i = 0; i < a->actor_header_count; i++) {
		actor_export_to_file(a->actor_headers + i, file);
	}
}

void collision_mesh_export_to_file(const struct collision_mesh *cm, FILE *file)
{
	u16 i, j, k;

	fwrite_ef16(&cm->triangle_count, file);
	for (i = 0; i < cm->triangle_count; i++) {
		struct collision_triangle *tri = cm->triangles + i;

		for (j = 0; j < 3; j++) {
			for (k = 0; k < 3; k++) {
				fwrite_ef32(tri->verts[j].pos + k, file);
			}
		}

		for (j = 0; j < 3; j++) {
			fwrite_ef32(tri->norm + j, file);
		}
	}

	for (i = 0; i < 3; i++) {
		fwrite_ef32(cm->offset.v + i, file);
	}
}

void actor_export_to_file(const struct actor_header *actor, FILE *file)
{
	u16 i;

	fwrite(actor->name, 1, ACTOR_NAME_MAX_LEN, file);

	for (i = 0; i < 3; i++) {
		fwrite_ef32(actor->position.v + i, file);
	}

	for (i = 0; i < 4; i++) {
		fwrite_ef32(actor->rotation.v + i, file);
	}

	for (i = 0; i < 3; i++) {
		fwrite_ef32(actor->scale.v + i, file);
	}
}
