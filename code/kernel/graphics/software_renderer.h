
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color_RGB_U8;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} Color_RGBA_U8;

typedef struct {
  uint64_t glyph_size;
  uint8_t *data;
} Bitmap_Font;

typedef struct {
  uint32_t width;
  uint32_t height;
  size_t stride;
  uint8_t *data;
} Software_Graphics_Buffer;

static uint8_t INCONSOLATA16_DATA[] = {
#include "INCONSOLATA16.txt"
};

static const Bitmap_Font INCONSOLATA16 = { 16, INCONSOLATA16_DATA };

void software_graphics_buffer_create(Software_Graphics_Buffer *software_graphics_buffer, uint32_t width, uint32_t height);
void software_graphics_buffer_draw_rectangle_solid(Software_Graphics_Buffer *software_graphics_buffer, int x, int y, int w, int h, Color_RGB_U8 color);
void software_graphics_buffer_overlay_buffers(Software_Graphics_Buffer *dest, Software_Graphics_Buffer *src, uint8_t alpha);
void software_graphics_draw_cstring(Software_Graphics_Buffer *dest, const Bitmap_Font *font, const char *cstring, int x, int y);