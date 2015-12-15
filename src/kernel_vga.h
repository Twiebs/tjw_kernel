#include <stdint.h>

static const uint16_t* VGA_MEMORY = (uint16_t*)0xB8000;
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

enum VGAColor {
	VGAColor_BLACK = 0,
	VGAColor_BLUE = 1,
	VGAColor_GREEN = 2,
	VGAColor_CYAN = 3,
	VGAColor_RED = 4,
	VGAColor_MAGENTA = 5,
	VGAColor_BROWN = 6
};

inline uint16_t vga_make_entry(char c, uint8_t color) 
{
	uint16_t char16 = c;
	uint16_t color16 = color;
	uint16_t result = char16 | color16 << 8;
	return result;
}

inline uint8_t vga_color(enum VGAColor foreground, enum VGAColor background) 
{
	uint8_t result = foreground | background << 4;
	return result;
}
