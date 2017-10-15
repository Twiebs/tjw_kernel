
#include <stdint.h>

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

void allay_get_framebuffer_info(Allay_Framebuffer_Info *framebuffer_info);
void allay_get_window_info(Allay_Window_Info *window_info);