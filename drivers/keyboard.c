#include "keyboard.h"
#include "stdio.h"

#define INPUT_BUFFER_SIZE 256

static const char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    
};

static char input_buffer[INPUT_BUFFER_SIZE];
static uint32_t buffer_index = 0;

void keyboard_init()
{
    // printf("Initializing keyboard...\n");

    uint8_t current_mask = inb(0x21);

    outb(0x21, current_mask & 0xFD);

    printf("Keyboard initialized (IRQ1 enabled)\n");
}

void keyboard_interrupt_handler()
{

    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    if (!(status & 0x01)) {
        // 没有数据，直接返回（安全措施）
        outb(0x20, 0x20);  // 发送EOI
        return;
    }

    uint8_t scancode = keyboard_read_scancode();

    if(scancode < 128) {
        char c = keyboard_scancode_to_ascii(scancode);
        if(c != 0) {
            keyboard_handle_input(c);
        }
    }

    outb(0x20, 0x20);
}

char keyboard_read_scancode()
{
    return inb(KEYBOARD_DATA_PORT);
}

char keyboard_scancode_to_ascii(uint8_t scancode)
{
    if(scancode >= 128) return 0;
    return keyboard_map[scancode];
}

void keyboard_handle_input(char c)
{
    if(c == '\b') {
        if(buffer_index > 0) {
            buffer_index--;
            put_char('\b', make_color(WHITE, BLACK));
            put_char(' ', make_color(WHITE, BLACK));
            put_char('\b', make_color(WHITE, BLACK));
        }
    }
    else if(c == '\n') {
        put_char('\n', make_color(WHITE, BLACK));

        if(buffer_index > 0) {
            input_buffer[buffer_index] = '\0';
            printf("You typed: %s\n", input_buffer);
            buffer_index = 0;
        }

        printf("os > ");
    }
    else {
        if(buffer_index < INPUT_BUFFER_SIZE -1 ) {
            input_buffer[buffer_index++] = c;
            put_char(c, make_color(WHITE, BLACK));
        }
    }
}
