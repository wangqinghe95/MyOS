#include "memory.h"

#include "screen.h"
#include "interrupt.h"

static uint32_t total_memory = 0;
static uint32_t usable_memory = 0;
static struct memory_region* memory_map = (struct memory_region*)(0x5000);
static uint32_t memory_map_entries = 0;

static uint8_t bitmap[BITMAP_SIZE] = {0};
static uint32_t total_frames = 0;
static uint32_t used_frames = 0;
static uint32_t bitmap_start_addr = 0;

/**
 * string operation
 * temporary realize
 * move to lib later
 */
static void memset(void* ptr, uint8_t value, uint32_t size)
{
    uint8_t* p = (uint8_t*)ptr;
    for(uint32_t i = 0; i < size; i++)
    {
        p[i] = value;
    }
}

void detect_memory(void)
{
    printf("Memory Configuration:\n");
    printf("  Compiled for: %d MB\n", KERNEL_MEMORY_MB);
    printf("  Total memory: %d MB\n", TOTAL_MEMORY / (1024*1024));
    printf("  Usable memory: %d MB (above 1MB)\n", USABLE_MEMORY / (1024*1024));
    printf("  Page size: %d bytes\n", PAGE_SIZE);
    printf("  Total pages: %d\n", USABLE_MEMORY / PAGE_SIZE);
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

    init_bitmap_allocator();
    // init_kernel_heap();

    printf("Memory management initialized for %d MB system\n", KERNEL_MEMORY_MB);
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

void init_bitmap_allocator(void)
{
    printf("Initializing bitmap allocator...\n");

    total_frames = USABLE_MEMORY / PAGE_SIZE;

    memset(bitmap, 0, BITMAP_SIZE);

    bitmap_start_addr = (uint32_t)&bitmap[0];
    uint32_t bitmap_end_addr = bitmap_start_addr + BITMAP_SIZE;

    uint32_t first_bitmap_frame = (bitmap_start_addr - USABLE_MEM_START) / PAGE_SIZE;
    uint32_t last_bitmap_frame = (bitmap_end_addr - USABLE_MEM_START) / PAGE_SIZE;

    for(uint32_t i = first_bitmap_frame; i <= last_bitmap_frame; i++)
    {
        if(i < total_frames){
            set_bitmap(i);
            used_frames++;
        }
    }

    printf("Bitmap allocator initialized:\n");
    printf("  Total frames: %d\n", total_frames);
    printf("  Bitmap size: %d bytes (%d pages)\n", BITMAP_SIZE, (last_bitmap_frame - first_bitmap_frame + 1));
    printf("  Used frames: %d\n", used_frames);
    printf("  Free frames: %d\n", total_frames - used_frames);
}
uint32_t allocate_frame(void)
{
    for(uint32_t i = 0; i < total_frames; i++)
    {
        if(!test_bitmap(i)) {
            set_bitmap(i);
            used_frames++;

            uint32_t physical_addr = USABLE_MEM_START + (i * PAGE_SIZE);

            return physical_addr;
        }
    }

    printf("Error: Out of memory! No free frames available.\n");
    return 0;
}

void free_frame(uint32_t frame_index)
{
    uint32_t index = (frame_index - USABLE_MEM_START) / PAGE_SIZE;
    if(index < total_frames) {
        if(test_bitmap(index)) {
            clear_bitmap(index);
            used_frames--;
        }
        else {
            printf("WARNING: Double free detected at frame %d\n", index);
        }
    }
    else {
        printf("ERROR: Invalid frame index %d\n", index);
    }
}
void print_bitmap_stats(void)
{
    uint32_t free_frames = total_frames - used_frames;
    uint32_t free_memory = free_frames * PAGE_SIZE;
    uint32_t used_memory = used_frames * PAGE_SIZE;
    
    printf("Memory Statistics:\n");
    printf("  Total frames: %d (%d MB)\n", total_frames, total_frames * PAGE_SIZE / (1024*1024));
    printf("  Used frames: %d (%d KB)\n", used_frames, used_memory / 1024);
    printf("  Free frames: %d (%d KB)\n", free_frames, free_memory / 1024);
    printf("  Memory usage: %d%%\n", (used_frames * 100) / total_frames);
}

void init_kernel_heap(void)
{
    printf("Kernel heap: TODO - starting at 0x%x\n", KERNEL_HEAP_START);
}

void set_bitmap(uint32_t bit)
{
    uint32_t byte_index = bit  / BITS_PER_BYTE;
    uint32_t bit_index = bit % BITS_PER_BYTE;

    if(byte_index < BITMAP_SIZE) {
        bitmap[byte_index] |= (1 << bit_index);
    }
}

void clear_bitmap(uint32_t bit)
{
    uint32_t byte_index = bit  / BITS_PER_BYTE;
    uint32_t bit_index = bit % BITS_PER_BYTE;

    if(byte_index < BITMAP_SIZE) {
        bitmap[byte_index] &= ~(1 << bit_index);
    }    
}

uint32_t test_bitmap(uint32_t bit)
{
    uint32_t byte_index = bit  / BITS_PER_BYTE;
    uint32_t bit_index = bit % BITS_PER_BYTE;

    if(byte_index < BITMAP_SIZE) {
        return (bitmap[byte_index] >> bit_index) & 1;
    }

    return 0;
}
