#ifndef STDIO_H
#define STDIO_H

#include "types.h"
#include "stdarg.h"
#include "screen.h"

int printf(const char* format, ...);
int putchar(int c);
int puts(const char* str);

int sprintf(char* buffer, const char* format, ...);
int vsprintf(char* buffer, const char* format, va_list args);

void itoa(int value, char* str, int base);
char* strcpy(char* dest, const char* src);
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
void memset(void* ptr, uint8_t value, uint32_t size);

void test_stdio_functions(void);
#endif