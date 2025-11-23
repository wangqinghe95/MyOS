#include "interrupt.h"
#include "screen.h"
#include "memory.h"
#include "timer.h"
#include "keyboard.h"

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
    printf("System ready with %d MB memory\n", get_kernel_memory_mb());
    
    // 启用中断
    asm volatile("sti");
    
    printf("Type 'help' for available commands\n");
    printf("os> ");
    
    while(1) {
        asm volatile("hlt");
    }
}