
#include <stddef.h>
#include <stdint.h>


extern void syscall_print_string(const char *string, uint64_t length);

#define syscall_print_literal(literal) syscall_print_string(literal, sizeof(literal) - 1)

int main() {
  syscall_print_literal("Hello kernel!  This is a message from userspace.");
  return 0;
}