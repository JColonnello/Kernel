#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stddef.h>

void * memset(void * destination, char c, size_t length);
void * memcpy(void * destination, const void * source, size_t length);
void *kmap(void **virtual, const void *hint, void **physical, size_t pageCount);
void libInit();
char *cpuVendor(char *result);
void _halt();

#endif