#include <stdio.h>

#include "endian.h"

u64 fwrite_ef16(const u16 *ptr, FILE *file)
{
	u16 flip = ((*ptr & 0x00FF) << 8) | ((*ptr & 0xFF00) >> 8);

	return fwrite(&flip, 2, 1, file);
}

u64 fwrite_ef32(const float *ptr, FILE *file)
{
	u32 bytes = *((u32 *)ptr);

	bytes = ((bytes & 0x000000FF) << 24) | ((bytes & 0x0000FF00) << 8) |
		((bytes & 0x00FF0000) >> 8) | ((bytes & 0xFF000000) >> 24);

	return fwrite(((float *)&bytes), 4, 1, file);
}

#if 0
u64 fread_ef16(u16 *ptr, FILE *file)
{
	u64 ret = fread(ptr, 2, 1, file);
	u16 bytes = *((u16 *)ptr);

	bytes = ((bytes & 0x00FF) << 8) | ((bytes & 0xFF00) >> 8);
	*ptr = *((u16 *)&bytes);

	return ret;
}

u64 fread_ef32(float *ptr, FILE *file)
{
	u64 ret = fread(ptr, sizeof *ptr, 1, file);
	u32 bytes = *((u32 *)ptr);

	bytes = ((bytes & 0x000000FF) << 24) | ((bytes & 0x0000FF00) << 8) |
		((bytes & 0x00FF0000) >> 8) | ((bytes & 0xFF000000) >> 24);
	*ptr = *((float *)&bytes);

	return ret;
}
#endif /* 0 */
