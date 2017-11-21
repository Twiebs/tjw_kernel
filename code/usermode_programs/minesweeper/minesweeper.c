
#include <allay_graphics.h>
#include <stddef.h>

int _start() {

  while (1) {
  	Allay_Framebuffer_Info framebuffer_info = {};
  	Allay_Window_Info window_info = {};
  	allay_get_framebuffer_info(&framebuffer_info);
  	allay_get_window_info(&window_info);

  	for (size_t y = window_info.y; y < window_info.y + window_info.height; y++) {
  	  for (size_t x = window_info.x; x < window_info.x + window_info.width; x++) {
  	  	size_t framebuffer_index = (y * framebuffer_info.stride) + x;
  	  	framebuffer_info.data[framebuffer_index + 0] = 0xFF;
  	  	framebuffer_info.data[framebuffer_index + 1] = 0xFF;
  	  	framebuffer_info.data[framebuffer_index + 2] = 0xFF;
  	  }
  	}
  }

  return 0;
}