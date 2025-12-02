#include "heap.h"
#include "memory.h"
#include "stdio.h"
#include "string.h"

static struct heap_block_header* heap_start = NULL;
static struct heap_block_header* heap_end = NULL;
static uint32_t heap_total_size = 0;
static uint32_t heap_used_size = 0;
static uint32_t total_allocations = 0;
static uint32_t total_frees = 0;


void heap_init(void)
{
    printf("Initializing kernel heap...\n");

    heap_start = (struct heap_block_header*)HEAP_START;

    heap_start->size = HEAP_INIT_SIZE;
    heap_start->used = 0;
    heap_start->next = NULL;
    heap_start->prev = NULL;

    heap_end = heap_start;
    heap_total_size = HEAP_INIT_SIZE;
    heap_used_size = sizeof(struct heap_block_header);

    HEAP_DEBUG("Heap initialized at 0x%x", HEAP_START);
    HEAP_DEBUG("Initial heap size: %d KB", HEAP_INIT_SIZE / 1024);
    HEAP_DEBUG("First block size: %d bytes", heap_start->size);
}

static void split_block(struct heap_block_header* block, uint32_t size)
{
    uint32_t remaining_size = block->size - size;

    if(remaining_size > sizeof(struct heap_block_header) + HEAP_ALIGNMENT) {
        struct heap_block_header* new_block = 
                    (struct heap_block_header*)((uint8_t*)block + size);

        new_block->size = remaining_size;
        new_block->used = 0;
        new_block->next = block->next;
        new_block->prev = block;

        if(block->next) {
            block->next->prev = new_block;
        }

        block->next = new_block;
        block->size = size;

        if(heap_end == block) {
            heap_end = new_block;
        }

        HEAP_DEBUG("Split block: 0x%x ->0x%x (%d bytes) and 0x%x (%d bytes)",
                block, block, block->size, new_block, new_block->size);
    }
}

static int heap_expand(uint32_t size)
{
    if(heap_total_size >= HEAP_MAX_SIZE) {
        printf("HEAP ERROR: Maximum heap size reached!\n");
        return 0;
    }

    uint32_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t expand_size = pages_needed * PAGE_SIZE;

    if(heap_total_size + expand_size > HEAP_MAX_SIZE) {
        printf("HEAP ERROR: Expansion would exceed max heap size\n");
        return 0;
    }

    uint32_t new_frame = allocate_frame();
    if(!new_frame) {
        printf("HEAP ERROR: Out of physical memory during expandsion\n");
        return 0;
    }

    struct heap_block_header* new_block = (struct heap_block_header*)new_frame;
    new_block->size = expand_size;
    new_block->used = 0;
    new_block->next = NULL;
    new_block->prev = heap_end;

    if(heap_end) {
        heap_end->next = new_block;
    }

    heap_end = new_block;
    heap_total_size += expand_size;

    HEAP_DEBUG("Heap expanded by %d  bytes at 0x%x", expand_size, new_frame);
    return 1;
}

void* kmalloc(uint32_t size)
{
    if(0 == size) return NULL;

    uint32_t total_size = ALIGN(size + sizeof(struct heap_block_header));
    HEAP_DEBUG("kmalloc requesst: %d bytes -> %bytes with header", size, total_size);
    
    struct heap_block_header* current = heap_start;
    while (current)
    {
        if(!current->used && current->size >= total_size) {
            HEAP_DEBUG("Found free block at 0x%x, size: %d bytes", current, current->size);

            if(current->size >= total_size + sizeof(struct heap_block_header) + HEAP_ALIGNMENT) {
                split_block(current, total_size);
            }

            current->used = 1;
            heap_used_size += current->size;
            total_allocations++;

            void* ptr = (void*)((uint8_t*)current + sizeof(struct heap_block_header));
            HEAP_DEBUG("Allocated %d bytes at 0x%x", size, ptr);

            return ptr;
        }

        current = current->next;
    }

    HEAP_DEBUG("No suitable block found, expanding heap...");

    if(heap_expand(total_size)) {
        return kmalloc(size);
    }

    printf("HEAP ERROR: Out of memory for allocation of %d bytes\n", size);
    return NULL;    
}

static void merge_free_block(struct heap_block_header* block)
{
    if(block->next && !block->next->used) {
        HEAP_DEBUG("Merging 0x%x with next block 0x%x", block, block->next);

        block->size += block->next->size;
        block->next = block->next->next;

        if(block->next) block->next->prev = block;
        else heap_end = block;
    }

    if(block->prev && !block->prev->used) {
        HEAP_DEBUG("Merging 0x%x with previous block 0x%x", block->prev, block);

        block->prev->size += block->size;
        block->prev->next = block->next;

        if(block->next) {
            block->next->prev = block->prev;
        }
        else {
            heap_end = block->prev;
        }

        block = block->prev;
    }
}

void kfree(void* ptr)
{
    if(!ptr) return;

    struct heap_block_header* header = 
        (struct heap_block_header*)((uint8_t*)ptr - sizeof(struct heap_block_header));


    if(!header->used) {
        printf("HEAP WARNING: Double free detected at 0x%x\n", ptr);
        return;
    }

    HEAP_DEBUG("Freeing block at 0x%x (header: 0x%x, size: %d bytes)",
                ptr, header, header->size);

    header->used = 0;
    heap_used_size -= header->size;
    total_frees++;

    merge_free_block(header);

}

void heap_dump(void)
{
    printf("\n=== Heap Dump ===\n");
    
    struct heap_block_header* current = heap_start;
    uint32_t block_count = 0;
    uint32_t used_blocks = 0;
    uint32_t free_blocks = 0;
    
    while (current) {
        printf("Block %d: 0x%x [%s] size: %d bytes\n",
               block_count, 
               (uint8_t*)current + sizeof(struct heap_block_header),
               current->used ? "USED" : "FREE",
               current->size - sizeof(struct heap_block_header));
        
        if (current->used) used_blocks++;
        else free_blocks++;
        
        block_count++;
        current = current->next;
    }
    
    printf("Total blocks: %d (Used: %d, Free: %d)\n", 
           block_count, used_blocks, free_blocks);
}

void heap_stats(void)
{
    uint32_t free_memory = heap_total_size - heap_used_size;
    uint32_t efficiency = (heap_total_size > 0) ? 
                         (heap_used_size * 100) / heap_total_size : 0;
    
    printf("\n=== Heap Statistics ===\n");
    printf("Total heap size:    %d KB\n", heap_total_size / 1024);
    printf("Used memory:        %d KB\n", heap_used_size / 1024);
    printf("Free memory:        %d KB\n", free_memory / 1024);
    printf("Memory efficiency:  %d%%\n", efficiency);
    printf("Total allocations:  %d\n", total_allocations);
    printf("Total frees:        %d\n", total_frees);
    printf("Active allocations: %d\n", total_allocations - total_frees);
}
