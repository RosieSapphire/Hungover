#include <malloc.h>

#include "util.h"
#include "animation.h"

/**
 * _anim_debug - Debug an Animation
 * @a: Animation to Debug
 */
static void _anim_debug(const struct animation *a)
{
	const char *fmt =
	"\tname=%s, mesh_index=%d, length=%d, npos=%d, nrot=%d, nsca=%d\n";
	printf(fmt, a->name, a->mesh_index, a->length,
		a->num_pos, a->num_rot, a->num_sca);

	for (u16 i = 0; i < a->num_pos; i++)
		printf("\t\tpos%d=(%f, %f, %f)\n", a->pos[i].frame,
				a->pos[i].vec[0], a->pos[i].vec[1],
				a->pos[i].vec[2]);

	printf("\n");

	for (u16 i = 0; i < a->num_rot; i++)
		printf("\t\trot%d=(%f, %f, %f, %f)\n", a->rot[i].frame,
				a->rot[i].vec[0], a->rot[i].vec[1],
				a->rot[i].vec[2], a->rot[i].vec[3]);

	printf("\n");

	for (u16 i = 0; i < a->num_sca; i++)
		printf("\t\tsca%d=(%f, %f, %f)\n", a->sca[i].frame,
				a->sca[i].vec[0], a->sca[i].vec[1],
				a->sca[i].vec[2]);

	printf("\n");
}

/**
 * anim_read - Animation Read from File
 * @a: Animation to read to
 * @file: File to read from
 */
void anim_read(struct animation *a, FILE *file)
{
	fread(a->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	fread(&a->mesh_index, sizeof(u16), 1, file);
	a->mesh_index = u16_endian_flip(a->mesh_index);
	fread(&a->length, sizeof(u16), 1, file);
	a->length = u16_endian_flip(a->length);
	fread(&a->num_pos, sizeof(u16), 1, file);
	a->num_pos = u16_endian_flip(a->num_pos);
	fread(&a->num_rot, sizeof(u16), 1, file);
	a->num_rot = u16_endian_flip(a->num_rot);
	fread(&a->num_sca, sizeof(u16), 1, file);
	a->num_sca = u16_endian_flip(a->num_sca);
	a->pos = malloc(sizeof(struct vec3_key) * a->num_pos);
	for (u16 i = 0; i < a->num_pos; i++)
	{
		fread(&a->pos[i].frame, sizeof(u16), 1, file);
		a->pos[i].frame = u16_endian_flip(a->pos[i].frame);
		fread(a->pos[i].vec, sizeof(f32), 3, file);
		for (u16 j = 0; j < 3; j++)
			a->pos[i].vec[j] = f32_endian_flip(a->pos[i].vec[j]);
	}
	a->rot = malloc(sizeof(struct vec4_key) * a->num_rot);
	for (u16 i = 0; i < a->num_rot; i++)
	{
		fread(&a->rot[i].frame, sizeof(u16), 1, file);
		a->rot[i].frame = u16_endian_flip(a->rot[i].frame);
		fread(a->rot[i].vec, sizeof(f32), 4, file);
		for (u16 j = 0; j < 4; j++)
			a->rot[i].vec[j] = f32_endian_flip(a->rot[i].vec[j]);
	}
	a->sca = malloc(sizeof(struct vec3_key) * a->num_sca);
	for (u16 i = 0; i < a->num_sca; i++)
	{
		fread(&a->sca[i].frame, sizeof(u16), 1, file);
		a->sca[i].frame = u16_endian_flip(a->sca[i].frame);
		fread(a->sca[i].vec, sizeof(f32), 3, file);
		for (u16 j = 0; j < 3; j++)
			a->sca[i].vec[j] = f32_endian_flip(a->sca[i].vec[j]);
	}
	_anim_debug(a);
}

/**
 * _anim_write_keyframes - Animation Write only keyframes to File
 * @a: Animation in Question
 * @file: File to write to
 */
static void _anim_write_keyframes(const struct animation *a, FILE *file)
{
	for (u16 i = 0; i < a->num_pos; i++)
	{
		u16 frame_flip = u16_endian_flip(a->pos[i].frame);
		f32 v[3] = {
			f32_endian_flip(a->pos[i].vec[0]),
			f32_endian_flip(a->pos[i].vec[1]),
			f32_endian_flip(a->pos[i].vec[2])
		};

		fwrite(&frame_flip, sizeof(u16), 1, file);
		fwrite(v, sizeof(f32), 3, file);
	}

	for (u16 i = 0; i < a->num_rot; i++)
	{
		u16 frame_flip = u16_endian_flip(a->rot[i].frame);
		f32 v[4] = {
			f32_endian_flip(a->rot[i].vec[0]),
			f32_endian_flip(a->rot[i].vec[1]),
			f32_endian_flip(a->rot[i].vec[2]),
			f32_endian_flip(a->rot[i].vec[3])
		};

		fwrite(&frame_flip, sizeof(u16), 1, file);
		fwrite(v, sizeof(f32), 4, file);
	}

	for (u16 i = 0; i < a->num_sca; i++)
	{
		u16 frame_flip = u16_endian_flip(a->sca[i].frame);
		f32 v[3] = {
			f32_endian_flip(a->sca[i].vec[0]),
			f32_endian_flip(a->sca[i].vec[1]),
			f32_endian_flip(a->sca[i].vec[2])
		};

		fwrite(&frame_flip, sizeof(u16), 1, file);
		fwrite(v, sizeof(f32), 3, file);
	}
}

/**
 * anim_write - Animation Write to File
 * @a: Animation in Question
 * @file: File to write to
 */
void anim_write(const struct animation *a, FILE *file)
{
	fwrite(a->name, sizeof(char), CONF_NAME_MAX_LEN, file);
	u16 mesh_index_flip = u16_endian_flip(a->mesh_index);
	u16 length_flip = u16_endian_flip(a->length);
	u16 num_pos_flip = u16_endian_flip(a->num_pos);
	u16 num_rot_flip = u16_endian_flip(a->num_rot);
	u16 num_sca_flip = u16_endian_flip(a->num_sca);

	fwrite(&mesh_index_flip, sizeof(u16), 1, file);
	fwrite(&length_flip, sizeof(u16), 1, file);
	fwrite(&num_pos_flip, sizeof(u16), 1, file);
	fwrite(&num_rot_flip, sizeof(u16), 1, file);
	fwrite(&num_sca_flip, sizeof(u16), 1, file);

	_anim_write_keyframes(a, file);
}
