#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define PAGE_SIZE (4096)
#define KERNEL_HEAP_START   (0x100000)
#define KERNEL_HEAP_SIZE    (0x100000)

#ifndef KERNEL_MEMORY_MB
#define KERNEL_MEMORY_MB 64
#endif

#define TOTAL_MEMORY (KERNEL_MEMORY_MB * 1024 * 1024)
#define USABLE_MEMORY (TOTAL_MEMORY - 0x100000)

struct memory_region
{
    uint32_t base_addr;
    uint32_t length;
    uint32_t type;
};


void memory_init(void);
uint32_t get_total_memory(void);
uint32_t get_usable_memory(void);
uint32_t get_kernel_memory_mb(void);

void* kmalloc(uint32_t size);
void kfree(void* ptr);

void set_bitmap(uint32_t bit);
void clear_bitmap(uint32_t bit);
uint32_t test_bitmap(uint32_t bit);

#endif