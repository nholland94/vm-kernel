#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include <stddef.h>

extern void kmalloc_init();
extern void *kmalloc(size_t);
extern void kfree(*void);

#endif
