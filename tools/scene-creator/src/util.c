#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <GL/glew.h>

#include "util.h"

int is_valid_path(const char *path)
{
	FILE *dummy = fopen(path, "rb");
	if (!dummy) {
		return 0;
	}

	fclose(dummy);
	return 1;
}

int file_extension_check(const char *path, const char *extension)
{
	const size_t pathlen = strlen(path);
	const size_t extlen = strlen(extension);

	if (extlen >= pathlen) {
		fprintf(stderr, "Extention '%s' is too long for file '%s'\n",
			extension, path);
		return 0;
	}

	return !strncmp(path + pathlen - extlen, extension, extlen);
}

/*********************
 * WRITING FUNCTIONS *
 *********************/
unsigned long fwrite_f32_flip(const float *val, FILE *file)
{
	uint32_t bytes = *((uint32_t *)val);

	bytes = ((bytes & 0x000000FF) << 24) | ((bytes & 0x0000FF00) << 8) |
		((bytes & 0x00FF0000) >> 8) | ((bytes & 0xFF000000) >> 24);

	float fval = *((float *)&bytes);

	return fwrite(&fval, sizeof val, 1, file);
}

unsigned long fwrite_u32_flip(const uint32_t *val, FILE *file)
{
	uint32_t bytes = *((uint32_t *)val);

	bytes = ((bytes & 0x000000FF) << 24) | ((bytes & 0x0000FF00) << 8) |
		((bytes & 0x00FF0000) >> 8) | ((bytes & 0xFF000000) >> 24);

	float fval = *((float *)&bytes);

	return fwrite(&fval, sizeof val, 1, file);
}

unsigned long fwrite_fvec_flip(const float *val, const int vecsize, FILE *file)
{
	unsigned long ret = 0;

	for (int i = 0; i < vecsize; i++) {
		ret += fwrite_f32_flip(val + i, file);
	}

	return ret;
}

unsigned long fwrite_u16_flip(const uint16_t *val, FILE *file)
{
	uint16_t fval = *val;

	fval = ((fval & 0x00FF) << 8) | ((fval & 0xFF00) >> 8);

	return fwrite(&fval, sizeof fval, 1, file);
}

/*********************
 * READING FUNCTIONS *
 *********************/
unsigned long fread_f32_flip(float *val, FILE *file)
{
	unsigned long ret = fread(val, sizeof *val, 1, file);
	uint32_t bytes = *((uint32_t *)val);

	bytes = ((bytes & 0x000000FF) << 24) | ((bytes & 0x0000FF00) << 8) |
		((bytes & 0x00FF0000) >> 8) | ((bytes & 0xFF000000) >> 24);
	*val = *((float *)&bytes);

	return ret;
}

unsigned long fread_fvec_flip(float *val, const int vecsize, FILE *file)
{
	unsigned long ret = 0;

	for (int i = 0; i < vecsize; i++) {
		ret += fread_f32_flip(val, file);
	}

	return ret;
}

unsigned long fread_u16_flip(uint16_t *val, FILE *file)
{
	unsigned long ret = fread(val, sizeof *val, 1, file);
	uint16_t bytes = *val;

	*val = ((bytes & 0x00FF) << 8) | ((bytes & 0xFF00) >> 8);

	return ret;
}
