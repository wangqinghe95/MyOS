#include "stdio.h"

int printf(const char* format, ...)
{
    char buffer[256];
    va_list args;

    va_start(args, format);
    int len = vsprintf(buffer, format, args);
    va_end(args);

    printk(buffer);

    return len;
}

int putchar(int c)
{
    put_char((char)c, make_color(WHITE, BLACK));
    return c;
}

int puts(const char* str)
{
    printk(str);
    put_char('\n', make_color(WHITE,BLACK));
    return strlen(str);
}


int sprintf(char* buffer, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vsprintf(buffer, format, args);
    va_end(args);
    return result;
}


int vsprintf(char* buffer, const char* format, va_list args)
{
    char* ptr = buffer;
    char num_buffer[32];
    char* str_arg;
    int int_arg;

    while (*format)
    {
        if(*format == '%') {
            format++;
            switch (*format)
            {
            case 'd':
            case 'i':
                int_arg = va_arg(args, int);
                itoa(int_arg, num_buffer, 10);
                strcpy(ptr, num_buffer);
                ptr += strlen(num_buffer);
                break;

            case 'u':
                int_arg = va_arg(args, int);
                itoa(int_arg, num_buffer, 10);
                strcpy(ptr, num_buffer);
                ptr += strlen(num_buffer);
                break;

            case 'x':
            case 'X':
                int_arg = va_arg(args, int);
                itoa(int_arg, num_buffer, 16);
                strcpy(ptr, num_buffer);
                ptr += strlen(num_buffer);
                break;

            case 'c':
                *ptr++ = (char)va_arg(args, int);
                break;

            case 's':
                str_arg = va_arg(args, char*);
                strcpy(ptr, str_arg);
                ptr += strlen(str_arg);
                break;

            case '%':
                *ptr++ = '%';
                break;

            default:
                *ptr++ = '%';
                *ptr++ = *format;
                break;
            }
        }
        else {
            *ptr++ = *format;
        }
        format++;
    }

    *ptr = '\0';
    return ptr - buffer;
}


void itoa(int value, char* str, int base)
{
    char* ptr = str;
    char* ptr1 = str;

    char tmp_char;
    int tmp_value;

    if(0 == value) {
        *ptr++ = '\0';
        *ptr = '\0';
        return;
    }

    while (value)
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
            
    }

    *ptr-- = '\0';

    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}


char* strcpy(char* dest, const char* src)
{
    char* ptr = dest;
    while (*src)
    {
        *ptr++ = *src++;
    }

    *ptr = '\0';

    return dest;    
}

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }

    return len;    
}

int strcmp(const char* s1, const char* s2)
{
    while (*s1 &&(*s1 == *s2))
    {
        s1++;
        s2++;
    }

    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}


void test_stdio_functions(void)
{
 char buffer[128];
    int result;
    
    printk_color("\n=== STDIO Function Tests ===\n", make_color(YELLOW, BLACK));
    
    /* 测试 strlen */
    result = strlen("");
    printf("strlen('') = %d %s\n", result, result == 0 ? "Yes" : "No");
    
    result = strlen("hello");
    printf("strlen('hello') = %d %s\n", result, result == 5 ? "Yes" : "No");
    
    /* 测试 strcpy */
    strcpy(buffer, "test");
    result = strcmp(buffer, "test");
    printf("strcpy -> '%s' %s\n", buffer, result == 0 ? "Yes" : "No");
    
    /* 测试 strcmp */
    result = strcmp("abc", "abc");
    printf("strcmp('abc', 'abc') = %d %s\n", result, result == 0 ? "Yes" : "No");
    
    result = strcmp("abc", "abd");
    printf("strcmp('abc', 'abd') = %d %s\n", result, result < 0 ? "Yes" : "No");
    
    /* 测试 itoa */
    itoa(123, buffer, 10);
    printf("itoa(123) -> '%s' %s\n", buffer, strcmp(buffer, "123") == 0 ? "Yes" : "No");
    
    itoa(-456, buffer, 10);
    printf("itoa(-456) -> '%s' %s\n", buffer, strcmp(buffer, "-456") == 0 ? "Yes" : "No");
    
    itoa(255, buffer, 16);
    printf("itoa(255, 16) -> '%s' %s\n", buffer, strcmp(buffer, "ff") == 0 ? "Yes" : "No");
    
    /* 测试 sprintf */
    sprintf(buffer, "Number: %d", 42);
    printf("sprintf -> '%s' %s\n", buffer, strcmp(buffer, "Number: 42") == 0 ? "Yes" : "No");
    
    sprintf(buffer, "Hex: 0x%x", 255);
    printf("sprintf hex -> '%s' %s\n", buffer, strcmp(buffer, "Hex: 0xff") == 0 ? "Yes" : "No");
    
    sprintf(buffer, "Char: %c", 'A');
    printf("sprintf char -> '%s' %s\n", buffer, strcmp(buffer, "Char: A") == 0 ? "Yes" : "No");
    
    sprintf(buffer, "String: %s", "test");
    printf("sprintf string -> '%s' %s\n", buffer, strcmp(buffer, "String: test") == 0 ? "Yes" : "No");
    
    /* 测试 printf */
    printf("printf test: %d %s %c 0x%x Yes\n", 123, "passed", 'A', 255);
    
    printk_color("=== STDIO Tests Completed ===\n", make_color(GREEN, BLACK));    
}
