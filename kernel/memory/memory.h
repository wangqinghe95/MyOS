#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define PAGE_SIZE (4096)

#define KERNEL_LOAD_ADDR  (0x10000)
#define USABLE_MEM_START    (0x100000)
#define KERNEL_HEAP_START   (0x100000)
#define KERNEL_HEAP_SIZE    (0x100000)

#ifndef KERNEL_MEMORY_MB
#define KERNEL_MEMORY_MB 64
#endif

#define TOTAL_MEMORY (KERNEL_MEMORY_MB * 1024 * 1024)
#define USABLE_MEMORY (TOTAL_MEMORY - 0x100000)

#define BITS_PER_BYTE 8
#define BITMAP_SIZE ((USABLE_MEMORY / PAGE_SIZE) / BITS_PER_BYTE + 1)

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

void init_bitmap_allocator(void);
uint32_t allocate_frame(void);
void free_frame(uint32_t frame_index);
void set_bitmap(uint32_t bit);
void clear_bitmap(uint32_t bit);
uint32_t test_bitmap(uint32_t bit);
void print_bitmap_stats(void);

void init_kernel_heap(void);

#endif