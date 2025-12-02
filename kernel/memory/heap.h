#ifndef HEAP_H
#define HEAP_H

#include "types.h"

#define HEAP_START  (0x100000)
#define HEAP_INIT_SIZE  (0x100000)
#define HEAP_MAX_SIZE  (0x100000)

struct heap_block_header
{
    uint32_t size;
    uint8_t used;
    struct heap_block_header* next;
    struct heap_block_header* prev;
};

#define HEAP_ALIGNMENT  8
#define ALIGN(size) (((size) + (HEAP_ALIGNMENT - 1)) & ~(HEAP_ALIGNMENT - 1))

void heap_init(void);
void* kmalloc(uint32_t size);
void kfree(void* ptr);
void heap_dump(void);
void heap_stats(void);

#define HEAP_DEBUG(mgs, ...)


#endif