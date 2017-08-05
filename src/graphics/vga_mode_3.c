
static const uint8_t VGA_TEXT_COLUMN_COUNT = 80;
static const uint8_t VGA_TEXT_ROW_COUNT = 25;
static uint8_t *VGA_TEXT_BUFFER = (uint8_t *)0xB8000;

void vga_set_char(char c, VGA_Color color, int x, int y) {
  size_t vga_index = ((y * VGA_TEXT_COLUMN_COUNT) + x) * 2;
  VGA_TEXT_BUFFER[vga_index + 0] = c;
  VGA_TEXT_BUFFER[vga_index + 1] = color;
}

void vga_clear_screen() {
  memory_set(VGA_TEXT_BUFFER, 0x00, VGA_TEXT_COLUMN_COUNT * VGA_TEXT_ROW_COUNT * 2);
}
