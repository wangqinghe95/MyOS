#include "screen.h"
#include "interrupt.h"

/* 触发除零异常的函数 */
void test_divide_by_zero(void) {
    printf("About to trigger divide by zero...\n");
    
    // 给用户一点时间看消息
    for (volatile int i = 0; i < 1000000; i++);
    
    // 触发除零异常
    // 纯 C 代码可能会直接被编译器优化掉
    asm volatile("mov $0, %eax");
    asm volatile("div %eax");  // 0 / 0 = 异常!
}

void kernel_main(void) {
    clear_screen();
    printf("Simple Interrupt System Test\n");
    printf("============================\n\n");
    
    // 初始化中断系统
    idt_init();
    
    printf("\nTesting divide by zero exception...\n");
    printf("This will trigger in 2 seconds...\n");
    
    // 简单延时
    for (volatile int i = 0; i < 3000000; i++);
    
    // 触发异常
    test_divide_by_zero();
    
    // 这行代码不应该执行
    printf("ERROR: If you see this, exception handling failed!\n");
    
    while(1) {
        // 空循环
    }
}