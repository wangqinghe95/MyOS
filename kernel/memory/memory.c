#include "memory.h"

#include "screen.h"
#include "interrupt.h"

static uint32_t total_memory = 0;
static uint32_t usable_memory = 0;
static struct memory_region* memory_map = (struct memory_region*)(0x5000);
static uint32_t memory_map_entries = 0;

void detect_memory(void)
{
    printf("Memory Configuration:\n");
    printf("  Compiled for: %d MB\n", KERNEL_MEMORY_MB);
    printf("  Total memory: %d MB\n", TOTAL_MEMORY / (1024*1024));
    printf("  Usable memory: %d MB (above 1MB)\n", USABLE_MEMORY / (1024*1024));
    printf("  Page size: %d bytes\n", PAGE_SIZE);
    
    // 显示内存布局
    printf("Memory Layout:\n");
    printf("  0x00000000 - 0x000FFFFF: BIOS, VGA, Kernel (1MB)\n");
    printf("  0x00100000 - 0x%08X: Usable Memory (%d MB)\n", 
           TOTAL_MEMORY - 1, USABLE_MEMORY / (1024*1024));   
}

void memory_init(void)
{
    printf("Initializing memory management...\n");

    detect_memory();

    if (KERNEL_MEMORY_MB < 16) {
        printf("WARNING: Memory configuration < 16MB may be insufficient\n");
    }
    
    if (KERNEL_MEMORY_MB > 512) {
        printf("WARNING: Memory configuration > 512MB may be unrealistic\n");
    }

    // init_bitmap_allocator();
    // init_kernel_heap();

    printf("Memory magagement initialized\n");
}

uint32_t get_total_memory(void)
{
    return total_memory;
}

uint32_t get_usable_memory(void)
{
    return USABLE_MEMORY;
}

uint32_t get_kernel_memory_mb(void)
{
    return KERNEL_MEMORY_MB;
}

void* kmalloc(uint32_t size)
{

}

void kfree(void* ptr)
{

}


void set_bitmap(uint32_t bit)
{

}

void clear_bitmap(uint32_t bit)
{

}

uint32_t test_bitmap(uint32_t bit)
{

}
