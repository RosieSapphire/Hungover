#ifndef _GLB_TO_SCN_ENDIAN_H_
#define _GLB_TO_SCN_ENDIAN_H_

#ifndef IS_USING_SCENE_CONVERTER
#define IS_USING_SCENE_CONVERTER
#endif /* IS_USING_SCENE_CONVERTER */

#include <stdio.h>

#include "../../include/types.h"

u64 fwrite_ef16(const u16 *ptr, FILE *file);
u64 fwrite_ef32(const float *ptr, FILE *file);
u64 fread_ef16(u16 *ptr, FILE *file);
u64 fread_ef32(float *ptr, FILE *file);

#endif /* _GLB_TO_SCN_ENDIAN_H_ */
