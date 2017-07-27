typedef enum {
  VGAColor_BLACK = 0,
  VGAColor_BLUE  = 1,
  VGAColor_GREEN = 2,
  VGAColor_CYAN  = 3,
  VGAColor_RED   = 4,
  VGAColor_MAGENTA = 5,
  VGAColor_BROWN = 6
} VGAColor;

typedef struct {
	const char *top_entry;
	uint32_t top_entry_index;
	int32_t scroll_count;
	uint32_t last_entry_count;
} VGA_Text_Terminal;

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t pitch;
  uint8_t  depth;
  uint8_t *buffer; 
} Framebuffer;

void kgfx_draw_log_if_dirty(Circular_Log *log);
void kgfx_draw_character(char c, size_t x_orign, size_t y_origin, Framebuffer *fb);