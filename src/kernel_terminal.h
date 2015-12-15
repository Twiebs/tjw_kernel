#include <stdint.h>
#include <stddef.h>

typedef struct {
	size_t Row;
	size_t Column;
	uint8_t Color;
	uint16_t* Buffer;
} terminal;

void terminal_initialize(terminal *Terminal) {
	Terminal->Row = 0;
	Terminal->Column = 0;
	Terminal->Color = vga_color(VGAColor_GREEN, VGAColor_BLACK);
	Terminal->Buffer = (uint16_t*)VGA_MEMORY; 
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = (y * VGA_WIDTH) + x;
			Terminal->Buffer[index] = vga_make_entry(' ', Terminal->Color);
		}
	}
}

void terminal_write_char(terminal *Terminal, char c) {
	const size_t Index = (Terminal->Row * VGA_WIDTH) + Terminal->Column;
	Terminal->Buffer[Index] = vga_make_entry(c, Terminal->Color);
	Terminal->Column += 1;
	if (Terminal->Column >= VGA_WIDTH) {
		Terminal->Column = 0;
		if (Terminal->Row == VGA_HEIGHT) {
			Terminal->Row = 0;
		}
	}	
}

size_t string_length(const char* String) {
	size_t Result = 0;
	while (String[Result] != 0) Result++;
	return Result;
}


void terminal_newline(terminal *Terminal) {
	Terminal->Row += 1;
	Terminal->Column = 0;
}

void terminal_write_string(terminal *Terminal, const char* Data) {
	size_t Length = string_length(Data);
	for (size_t i = 0; i < Length; i++) {
		if (Data[i] == '\n') {
			terminal_newline(Terminal);
			continue;
		}
		terminal_write_char(Terminal, Data[i]);
	}
}
