#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stddef.h>
#define PAGE_SIZE 0x1000
#define PAGE_MASK (uintptr_t)~0xFFF

void * memset(void * destination, char c, size_t length);
void * memcpy(void * destination, const void * source, size_t length);
void *kmap(void **virtual, const void *hint, void **physical, size_t pageCount);
void kunmap(void *virtual, size_t pageCount);
void *kmalloc(size_t size);
void *kcalloc(size_t nmemb, size_t size);
void kfree(void *ptr);
uintptr_t createPML4();
void libInit();
char *cpuVendor(char *result);
void _halt();

#endif