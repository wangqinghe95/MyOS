#ifndef LOGGING_H
#define LOGGING_H

#include "types.h"
#include "stdarg.h"

typedef enum
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} log_level_t;

void log_init();
void log_message(log_level_t level, const char* tag, const char* format, ...);
void log_hex_dump(const char* tag, const void* data, uint32_t size);

#define log_debug(tag, fmt, ...) log_message(LOG_DEBUG, tag, fmt, ##__VA_ARGS__)
#define log_info(tag, fmt, ...) log_message(LOG_INFO, tag, fmt, ##__VA_ARGS__)
#define log_warn(tag, fmt, ...) log_message(LOG_WARN, tag, fmt, ##__VA_ARGS__)
#define log_error(tag, fmt, ...) log_message(LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#define log_fatal(tag, fmt, ...) log_message(LOG_FATAL, tag, fmt, ##__VA_ARGS__)

#define TEST_ASSERT(condition, message) \
    do  {   \
        if(!(condition)) {  \
            log_fatal("TEST", "FATAL: %s (at %s:%d)", message, __FILE__, __LINE__); \
            while (1) asm volatile("hlt");  \
        }   \
        else {  \
            log_info("TEST", "PASSED: %s", message); \
        }   \
    } while(0)

#define LOG(msg) log_info("LOG", "%s", msg)
#define LOG_FMT(fmt, ...) log_info("LOG", fmt, ##__VA_ARGS__)
#define TEST_PHASE(title) log_info("TEST", "=== %s ===", title)
#define TEST_SUCCESS(name) log_info("TEST", "%s PASSED", name)

void test_logging_system();

#endif