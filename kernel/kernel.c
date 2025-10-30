void kernel_main(void) {
    char *vga = (char*)0xB8000;
    
    // 清屏
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vga[i] = ' ';
        vga[i + 1] = 0x07;
    }
    
    // 显示字符串
    char *msg = "Hello from C Kernel!";
    for (int i = 0; msg[i]; i++) {
        vga[i * 2] = msg[i];
        vga[i * 2 + 1] = 0x07;
    }
    
    while(1) asm("hlt");
}