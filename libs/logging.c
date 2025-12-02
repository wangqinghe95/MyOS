#include "logging.h"
#include "stdio.h"

static const char* level_name[] = {"DEBUG", "INFO", "WARN", "EROR", "FATAL"};
static const uint8_t level_colors[] = {
    LIGHT_GRAY, WHITE, YELLOW, LIGHT_RED, RED
};

void log_init()
{
    log_debug("LOGGING", "Logging system initialized");
}

void log_message(log_level_t level, const char* tag, const char* format, ...)
{
    char buffer[256];
    va_list args;
    uint8_t color = make_color(level_colors[level], BLACK);

    char* ptr = buffer;
    ptr += sprintf(ptr, "[%5s] %8s: ", level_name[level], tag);

    va_start(args, format);
    ptr += vsprintf(ptr, format, args);
    va_end(args);

    printk_color(buffer, color);
    put_char('\n', color);
}
#if 0
void log_hex_dump(const char* tag, const void* data, uint32_t size)
{
    const uint8_t *bytes = (const uint8_t*)data;

    log_info(tag, "Hex dump (%d bytes):", size);

    for(uint32_t i = 0; i < size; i += 16) {
        char line[80];
        char* ptr = line;

        ptr += sprintf(ptr, "  %04x: ", i);

        for(uint32_t j = 0; j < 16; j++) {
            if(i + j < size) ptr += sprintf(ptr, "%02x ", bytes[i+j]);
            else ptr += sprintf(ptr, "    ");
            if(7 == j) ptr += sprintf(ptr, " ");
        }

        ptr += sprintf(ptr, " |");
        for(uint32_t j = 0; j < 16 && i + j < size; j++) {
            char c = bytes[i + j];
            *ptr++ = (c >= 32 && c < 127) ? c : '.';
        }

        *ptr++ = '|';
        *ptr = '\0';

        log_debug("HEX", "%s", line);
    }
}
#else

void log_hex_dump(const char* tag, const void* data, uint32_t size) {
    const uint8_t* bytes = (const uint8_t*)data;
    char line[80];
    
    // 输出头部信息
    sprintf(line, "[%s] Hex dump (%u bytes):", tag, size);
    printf("%s\n", line);
    
    for (uint32_t i = 0; i < size; i += 16) {
        char* ptr = line;
        uint32_t line_bytes = (size - i) < 16 ? 
                            (size - i) : 16;
        
        // 地址部分
        ptr += sprintf(ptr, "  %04x: ", i);
        
        // 十六进制部分
        for (uint32_t j = 0; j < 16; j++) {
            if (j < line_bytes) {
                ptr += sprintf(ptr, "%02x ", bytes[i + j]);
            } else {
                ptr += sprintf(ptr, "   ");
            }
            if (j == 7) {
                ptr += sprintf(ptr, " ");
            }
        }
        
        // ASCII部分
        ptr += sprintf(ptr, "|");
        for (uint32_t j = 0; j < line_bytes; j++) {
            char c = bytes[i + j];
            *ptr++ = (c >= 32 && c < 127) ? c : '.';
        }
        *ptr++ = '|';
        *ptr = '\0';
        
        printf("%s\n", line);
    }
}

void test_hex_dump() {
    // 测试字符串
    char test_str[] = "This is a test string for hex dump!\x01\x02\x7F\xFF";
    log_hex_dump("TEST_STR", test_str, sizeof(test_str));
    
    // 测试二进制数据
    uint8_t binary_data[] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
        0x01, 0x02, 0x03, 0x04
    };
    log_hex_dump("BINARY", binary_data, sizeof(binary_data));
}

#endif
void test_logging_system()
{
    printk_color("\n=== LOGGING System Tests ===\n", make_color(YELLOW, BLACK));
    
    /* 测试不同日志级别 */
    log_debug("TEST", "This is a debug message");
    log_info("TEST", "This is an info message");
    log_warn("TEST", "This is a warning message");
    log_error("TEST", "This is an error message");
    
    /* 测试格式化日志 */
    log_info("TEST", "Format test: %d + %d = %d", 2, 3, 5);
    log_info("TEST", "String: %s, Char: %c, Hex: 0x%x", "hello", 'X', 255);
    
    /* 测试十六进制转储 */
    uint8_t test_data[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                          0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
                          'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
    
    // log_hex_dump("TEST", test_data, sizeof(test_data));
    // test_hex_dump();
    
    /* 测试断言宏 */
    TEST_ASSERT(1 == 1, "Basic assertion should pass");
    
    int x = 5, y = 5;
    TEST_ASSERT(x == y, "Variable comparison should pass");
    
    /* 测试日志宏 */
    LOG("Simple log message");
    LOG_FMT("Formatted log: %d, %s", 42, "test");
    TEST_PHASE("Test phase demonstration");
    TEST_SUCCESS("LOGGING SYSTEM");
    
    printk_color("=== Logging Tests Completed ===\n", make_color(GREEN, BLACK));
}