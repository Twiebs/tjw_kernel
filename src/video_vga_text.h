
typedef enum {
  VGAColor_BLACK = 0,
  VGAColor_BLUE  = 1,
  VGAColor_GREEN = 2,
  VGAColor_CYAN  = 3,
  VGAColor_RED   = 4,
  VGAColor_MAGENTA = 5,
  VGAColor_BROWN = 6
} VGATextColor;

typedef struct {
	const char *top_entry;
	uint32_t top_entry_index;
	int32_t scroll_count;
	uint32_t last_entry_count;
} VGATextTerm; 

internal void kterm_redraw_if_required(VGATextTerm *kterm, IOState *io);
