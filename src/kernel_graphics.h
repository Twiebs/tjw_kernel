typedef enum {
  VGA_Color_BLACK = 0,
  VGA_Color_BLUE  = 1,
  VGA_Color_GREEN = 2,
  VGA_Color_CYAN  = 3,
  VGA_Color_RED   = 4,
  VGA_Color_MAGENTA = 5,
  VGA_Color_BROWN = 6,
  VGA_Color_LIGHT_GRAY = 7,
  VGA_Color_DARK_GRAY = 8,
  VGA_Color_LIGHT_BLUE = 9,
  VGA_Color_LIGHT_GREEN = 10,
  VGA_Color_LIGHT_CYAN = 11,
  VGA_Color_LIGHT_RED = 12,
  VGA_Color_LIGHT_MAGENTA = 13,
  VGA_Color_YELLOW = 14,
  VGA_Color_WHITE = 15,
} VGA_Color;

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t pitch;
  uint8_t  depth;
  uint8_t *buffer; 
} Framebuffer;

void kgfx_draw_log_if_dirty(Circular_Log *log);
void kgfx_draw_character(char c, size_t x_orign, size_t y_origin, Framebuffer *fb);