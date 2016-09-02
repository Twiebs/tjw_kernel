
#include <stddef.h>
#include <stdint.h>


typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t pitch;
  uint8_t  depth;
  uint8_t *buffer; 
} Framebuffer;

#define invoke_syscall(n) asm volatile("mov $" #n ", %rax"); asm volatile("int $0x80")
#define define_syscall(n, name, ...) void syscall_##name(__VA_ARGS__) { invoke_syscall(n); } 

define_syscall(0, print_string, const char *string, uint64_t length)
define_syscall(2, get_framebuffer, Framebuffer *fb)

//extern void syscall_print_string(const char *string, uint64_t length);
#define syscall_print_literal(literal) syscall_print_string(literal, sizeof(literal) - 1)

#define abs(x) __builtin_fabsf(x)

#if 0
static const uint8_t WALLPAPER_DATA[] = {
#include "wallpaper.txt"
};
#endif

float lerp(float a, float b, float t){
  float result = a+((b-a)*t);
  return result;
}

int main() {
  syscall_print_literal("Hello kernel! This is a message from userspace.");

  Framebuffer framebuffer;
  syscall_get_framebuffer(&framebuffer);
  for(size_t y = 0; y < framebuffer.height; y++){
    for(size_t x = 0; x < framebuffer.width; x++){
      float t = (float)y / (float)framebuffer.height;

      float r = abs(t*2.0f+1.0f) * 0.5f;
      size_t index = ((x*framebuffer.depth) + y*framebuffer.pitch);
      framebuffer.buffer[index + 0] = 0x30; 
      framebuffer.buffer[index + 1] = 0x00; 
      framebuffer.buffer[index + 2] = (uint8_t)r; 
      framebuffer.buffer[index + 3] = 0xFF; 
    }
  }

   return 0;
}