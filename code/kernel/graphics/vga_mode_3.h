
static const uint8_t VGA_TEXT_COLUMN_COUNT = 80;
static const uint8_t VGA_TEXT_ROW_COUNT = 25;

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

void vga_set_char(char c, VGA_Color color, int x, int y);
void vga_write_string(const char *string, size_t length, VGA_Color color, int xoffset, int line);
void vga_clear_screen();