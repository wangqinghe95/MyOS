#include "screen.h"
#include "interrupt.h"

// ==================== 当前可用的测试函数 ====================

/**
 * 测试1：除零异常
 */
void test_divide_by_zero(void) {
    printf("=== Divide by Zero Test ===\n\n");
    printf("This test will trigger a division by zero exception.\n");
    printf("The system should catch the exception and display information.\n\n");
    
    printf("Triggering in 2 seconds...\n");
    for (volatile int i = 0; i < 2000000; i++); // 延时
    
    printf("Executing: 0 / 0\n");
    asm volatile("mov $0, %eax");
    asm volatile("div %eax");  // 💥 触发除零异常
    
    // 这行代码不会执行
    printf("ERROR: Divide by zero test failed!\n");
}

/**
 * 测试2：通用保护故障
 */
void test_general_protection_fault(void) {
    printf("=== General Protection Fault Test ===\n\n");
    printf("This test will trigger a segment protection violation.\n");
    printf("The system should catch the GPF and display error details.\n\n");
    
    printf("Triggering in 2 seconds...\n");
    for (volatile int i = 0; i < 2000000; i++); // 延时
    
    printf("Loading invalid segment register...\n");
    asm volatile("mov $0x1234, %eax");   // 无效的段选择子
    asm volatile("mov %eax, %ds");       // 💥 触发GPF
    
    // 这行代码不会执行
    printf("ERROR: General protection fault test failed!\n");
}

// ==================== 为后续开发预留的接口 ====================

/**
 * 内存相关异常测试 - 预留接口
 */
void test_memory_exceptions(void) {
    printf("=== Memory Exception Tests ===\n\n");
    printf("[To be implemented]\n");
    printf("Page faults, invalid memory access, etc.\n");
}

/**
 * 硬件中断测试 - 预留接口  
 */
void test_hardware_interrupts(void) {
    printf("=== Hardware Interrupt Tests ===\n\n");
    printf("[To be implemented]\n");
    printf("Timer, keyboard, and other hardware interrupts.\n");
}

/**
 * 运行测试
 */
void run_test(int choice) {
    clear_screen();
    
    switch(choice) {
        case 1:
            test_divide_by_zero();
            break;
            
        case 2:
            test_general_protection_fault();
            break;
            
        case 3:
            test_memory_exceptions();
            printf("\nPress any key to continue...\n");
            for (volatile int i = 0; i < 3000000; i++);
            break;
            
        case 4:
            test_hardware_interrupts();
            printf("\nPress any key to continue...\n");
            for (volatile int i = 0; i < 3000000; i++);
            break;
            
        case 5:
            // show_system_info();
            // printf("\nPress any key to continue...\n");
            // for (volatile int i = 0; i < 3000000; i++);
            break;
            
        case 6:
            // printf("=== Running All Available Tests ===\n\n");
            // test_divide_by_zero();
            // test_general_protection_fault() 不会执行，因为上面已经挂起了
            break;
            
        default:
            printf("Invalid selection!\n");
            for (volatile int i = 0; i < 2000000; i++);
            break;
    }
}

// ==================== 主函数 ====================

void kernel_main(void) {
    // 系统初始化
    clear_screen();
    printf("OS Interrupt System\n");
    printf("===================\n\n");
    
    // 初始化中断系统
    idt_init();
    printf("Interrupt system ready.\n\n");
    
    // 显示系统信息
    // show_system_info();
    
    // 简单延时后开始测试
    printf("Starting tests in 2 seconds...\n");
    for (volatile int i = 0; i < 2000000; i++);
    
    clear_screen();
    // test_divide_by_zero();
    run_test(2);
    
    // 如果异常处理失败，才会执行到这里
    printf("\nERROR: Exception handling failed! System unstable.\n");
    
    // 系统挂起
    while(1) {
        asm volatile("hlt");
    }
}