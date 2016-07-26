
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

void redraw_vga_text_terminal_if_dirty(VGA_Text_Terminal *term, Console_Buffer *cb);
