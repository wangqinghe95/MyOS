#include "screen.h"
#include "interrupt.h"

void kernel_main(void) {
    clear_screen();
    printf("=== Minimal Exception Test ===\n\n");
    
    // 显示关键地址信息
    printf("Key Addresses:\n");
    printf("  kernel_main: 0x%x\n", (uint32_t)kernel_main);
    
    // 初始化中断系统
    idt_init();
    
    printf("\nIf addresses are wrong, system will crash.\n");
    printf("If correct, you'll see 'EXCP0' in top-left.\n\n");
    
    // 触发测试
    test_interrupt();
    
    // 不应该执行到这里
    printf("TEST FAILED: Should not reach here!\n");
    while(1);
}