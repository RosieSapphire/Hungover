#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <stdint.h>

int is_valid_path(const char *path);
int file_extension_check(const char *path, const char *extension);

unsigned long fwrite_f32_flip(const float *val, FILE *file);
unsigned long fwrite_u32_flip(const uint32_t *val, FILE *file);
unsigned long fwrite_fvec_flip(const float *val, const int vecsize, FILE *file);
unsigned long fwrite_u16_flip(const uint16_t *val, FILE *file);

unsigned long fread_f32_flip(float *val, FILE *file);
unsigned long fread_u32_flip(uint32_t *val, FILE *file);
unsigned long fread_fvec_flip(float *val, const int vecsize, FILE *file);
unsigned long fread_u16_flip(uint16_t *val, FILE *file);

#endif /* _UTIL_H_ */
