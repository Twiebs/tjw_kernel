
#include <stddef.h>
#include <stdint.h>


extern void syscall_print_string(const char *string, uint64_t length);

#define syscall_print_literal(literal) syscall_print_string(literal, sizeof(literal) - 1)

int main() {
  asm volatile("xchgw %bx, %bx");
  syscall_print_literal("hello kernel!");
  return 0;
}