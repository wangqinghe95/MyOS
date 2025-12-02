#ifndef STDARG_H
#define STDARG_H

typedef char* va_list;

#define __va_align(type)    \
    (((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))


#define va_start(ap, last)  \
        ((ap) = (va_list)&(last) + __va_align(last))


#define va_arg(ap, type)    \
        (*(type*)((ap) += __va_align(type),(ap) - __va_align(type)))

#define va_end(ap) ((ap) = (va_list)0)

#endif