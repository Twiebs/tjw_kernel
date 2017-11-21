
#include <stdint.h>

typedef enum {
  Allay_Error_Code_NONE,
} Allay_Error_Code;

typedef struct {
  uint64_t width;
  uint64_t height;
  uint64_t depth;
  uint64_t stride;
  uint8_t *data;
} Allay_Framebuffer_Info;

typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
} Allay_Window_Info;

static inline Allay_Error_Code allay_get_framebuffer_info(Allay_Framebuffer_Info *framebuffer_info) {
  asm volatile("mov $3, %rax"); 
  asm volatile("int $0x80");
  return Allay_Error_Code_NONE;
}

static inline Allay_Error_Code allay_get_window_info(Allay_Window_Info *window_info) {
  asm volatile("mov $4, %rax"); 
  asm volatile("int $0x80");
  return Allay_Error_Code_NONE;
}