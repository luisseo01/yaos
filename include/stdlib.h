#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <types.h>

#include <kernel/page.h>

#ifndef CONFIG_PAGING
void *kmalloc(unsigned long size);
void free(void *addr);
#endif

void *malloc(size_t size);

void *memcpy(void *dst, const void *src, int len);
void *memset(void *src, int c, int n);

#endif /* __STDLIB_H__ */
