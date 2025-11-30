#include "interrupt.h"
#include "screen.h"
#include "memory.h"
#include "timer.h"
#include "keyboard.h"

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
    
    // 3. 初始化硬件驱动
    init_timer();
    keyboard_init();
    
    printf("\nKernel initialized successfully\n");
    
    test_memory_allocation();
    
    asm volatile("sti");

    printf("\nSystem ready with %d MB memory\n", get_kernel_memory_mb());
    printf("Type 'help' for available commands\n");
    printf("os> ");
    
    while(1) {
        asm volatile("hlt");
    }
}