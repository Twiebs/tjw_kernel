

static uint8_t *VGA_TEXT_BUFFER = (uint8_t *)0xB8000;

void vga_set_char(char c, VGA_Color color, int x, int y) {
  size_t vga_index = ((y * VGA_TEXT_COLUMN_COUNT) + x) * 2;
  VGA_TEXT_BUFFER[vga_index + 0] = c;
  VGA_TEXT_BUFFER[vga_index + 1] = color;
}

void vga_write_string(const char *string, size_t length, VGA_Color color, int xoffset, int line) {
  kassert(xoffset + length <= VGA_TEXT_COLUMN_COUNT);
  for (size_t i = 0; i < length; i++) {
    vga_set_char(string[i], color, xoffset + i, line);
  }
}

void vga_clear_screen() {
  memory_set(VGA_TEXT_BUFFER, 0x00, VGA_TEXT_COLUMN_COUNT * VGA_TEXT_ROW_COUNT * 2);
}
