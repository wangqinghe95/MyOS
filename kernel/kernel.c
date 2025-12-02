#include "interrupt.h"
#include "memory.h"
#include "timer.h"
#include "keyboard.h"
#include "heap.h"
#include "stdio.h"
#include "logging.h"

void test_memory_allocation(void) {
    printf("\n=== Memory Allocation Test ===\n");
    
    // 显示初始状态
    print_bitmap_stats();
    
    printf("\nAllocating test frames...\n");
    
    // 测试分配几个页面
    uint32_t frames[5];
    for (int i = 0; i < 5; i++) {
        frames[i] = allocate_frame();
        if (frames[i] != 0) {
            printf("  Allocated frame %d: 0x%x\n", i, frames[i]);
        }
    }
    
    // 显示分配后状态
    printf("\nAfter allocation:\n");
    print_bitmap_stats();
    
    printf("\nFreeing some frames...\n");
    
    // 释放部分页面
    for (int i = 1; i < 4; i++) {  // 释放中间3个
        if (frames[i] != 0) {
            free_frame(frames[i]);
            printf("  Freed frame: 0x%x\n", frames[i]);
            frames[i] = 0;
        }
    }
    
    // 显示最终状态
    printf("\nAfter freeing:\n");
    print_bitmap_stats();
    
    printf("\nTesting kmalloc (temporary implementation)...\n");
    void* ptr1 = kmalloc(1024);  // 申请1KB
    void* ptr2 = kmalloc(2048);  // 申请2KB
    
    if (ptr1 != 0 && ptr2 != 0) {
        printf("  kmalloc test passed: 0x%x, 0x%x\n", (uint32_t)ptr1, (uint32_t)ptr2);
        
        // 清理
        kfree(ptr1);
        kfree(ptr2);
    }
    
    printf("Memory test completed successfully!\n");
}

void test_heap_allocator()
{
    printf("\n=== Heap Allocator Test ===\n");

    heap_init();
    heap_stats();

    printf("\n1. Testing basic allocation..\n");

    void* ptr1 = kmalloc(64);
    void* ptr2 = kmalloc(128);
    void* ptr3 = kmalloc(512);
    void* ptr4 = kmalloc(32);

    if (ptr1 && ptr2 && ptr3 && ptr4) {
        printf("  ✓ Basic allocation successful\n");
        printf("  Allocated: 64B@0x%x, 128B@0x%x, 512B@0x%x, 32B@0x%x\n",
               ptr1, ptr2, ptr3, ptr4);
    } else {
        printf("  ✗ Basic allocation failed\n");
        return;
    }

    heap_stats();
    heap_dump();

    printf("\n2. Testing free and reuse...\n");
    kfree(ptr2);
    kfree(ptr4);

    printf("  Freed 128B and 32B blocks\n");
    
    // 分配新块（应该重用空闲块）
    void* ptr5 = kmalloc(100);   // 应该重用128字节的空闲块
    void* ptr6 = kmalloc(24);    // 应该重用32字节的空闲块
    
    if (ptr5 && ptr6) {
        printf("  ✓ Free and reuse successful\n");
        printf("  Reallocated: 100B@0x%x, 24B@0x%x\n", ptr5, ptr6);
    }
    
    heap_stats();
    heap_dump();
    
    printf("\n3. Testing boundary conditions...\n");
    
    // 测试0字节分配
    void* ptr_zero = kmalloc(0);
    printf("  kmalloc(0) = 0x%x %s\n", 
           ptr_zero, ptr_zero == NULL ? "✓" : "✗");
    
    // 测试NULL释放
    kfree(NULL);
    printf("  kfree(NULL) - no crash ✓\n");
    
    // 测试大分配（触发堆扩展）
    printf("  Testing large allocation (triggering heap expansion)...\n");
    void* large_ptr = kmalloc(8192);  // 8KB - 应该触发扩展
    
    if (large_ptr) {
        printf("  ✓ Large allocation successful: 8KB@0x%x\n", large_ptr);
        kfree(large_ptr);
    } else {
        printf("  ✗ Large allocation failed\n");
    }
    
    heap_stats();
    
    printf("\n4. Testing fragmentation...\n");
    
    // 创建碎片化模式：分配-释放交替
    void* frag_ptrs[6];
    for (int i = 0; i < 6; i++) {
        frag_ptrs[i] = kmalloc(64 * (i + 1));  // 64, 128, 192, 256, 320, 384
    }
    
    // 释放奇数索引的块
    for (int i = 1; i < 6; i += 2) {
        kfree(frag_ptrs[i]);
    }
    
    printf("  Created fragmentation pattern\n");
    heap_dump();
    
    // 尝试分配中等大小块（应该合并空闲块）
    void* merged_ptr = kmalloc(300);
    if (merged_ptr) {
        printf("  ✓ Block merging successful: 300B@0x%x\n", merged_ptr);
        kfree(merged_ptr);
    }
    
    // 清理所有分配
    for (int i = 0; i < 6; i += 2) {  // 只清理未释放的
        if (frag_ptrs[i]) kfree(frag_ptrs[i]);
    }
    
    heap_stats();
    heap_dump();
    
    printf("\n=== Heap Test Completed ===\n");
}

void kernel_main(void) {
    clear_screen();
    printf("MyOS Boot Start...\n");
    printf("=========================================\n\n");
    
    // 1. 初始化中断系统
    idt_init();
    init_pic();
    install_timer_interrupt();
    install_keyboard_interrupt();
    
    // 2. 初始化内存管理系统
    memory_init();
    
    test_heap_allocator();

    // 3. 初始化硬件驱动
    init_timer();
    keyboard_init();
    
    // test_stdio_functions();
    test_logging_system();

    printf("\nKernel initialized successfully\n");
    printf("System ready with %d MB memory\n", get_kernel_memory_mb());
    printf("Heap allocator active - type 'help' for commands\n");
    printf("os> ");
    
    // 启用中断
    asm volatile("sti");
    
    while(1) {
        asm volatile("hlt");
    }
}