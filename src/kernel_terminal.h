#include <stdint.h>
#include <stddef.h>

void terminal_initialize();
void terminal_write_string(const char *String);
void terminal_write_uint8_hex(uint8_t Value);
void terminal_write_uint32_hex(uint32_t Value);
void terminal_newline();
