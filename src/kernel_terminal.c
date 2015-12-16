#include "kernel_terminal.h"
#include "kernel_vga.h"

typedef struct {
	size_t Row;
	size_t Column;
	uint8_t Color;
	uint16_t* Buffer;
} terminal;

static terminal GlobalTerminal;

static size_t string_length(const char* String) {
	size_t Result = 0;
	while (String[Result] != 0) Result++;
	return Result;
}

static inline void initialize(terminal *Terminal) {
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

static inline void write_char(terminal *Terminal, char c) {
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


static inline void newline(terminal *Terminal) {
	Terminal->Row += 1;
	Terminal->Column = 0;
}

static inline void write_string(terminal *Terminal, const char* Data) {
	size_t Length = string_length(Data);
	for (size_t i = 0; i < Length; i++) {
		if (Data[i] == '\n') {
			newline(Terminal);
			continue;
		}
		write_char(Terminal, Data[i]);
	}
}

static inline void write_four_bits(terminal *Terminal, uint8_t bits)
{
	if (bits > 10) {
		write_char(Terminal, 65 + (bits - 10));
	} else {
		write_char(Terminal, 48 + bits);
	}
}

static inline void write_uint8_hex(terminal *Terminal, uint8_t Value)
{
	uint8_t Top = Value >> 4;
	uint8_t Bottom = (uint8_t)((uint8_t)(Value << 4) >> 4);
	write_four_bits(Terminal, Top);
	write_four_bits(Terminal, Bottom);
}


static inline void write_uint32_hex(terminal *Terminal, uint32_t Value) 
{
	write_string(Terminal, "0x");
		
	uint8_t Bytes[4];
	Bytes[0] = (uint8_t)(Value >> 24);
	Bytes[1] = (uint8_t)(Value >> 16);
	Bytes[2] = (uint8_t)(Value >> 8);
	Bytes[3] = (uint8_t)(Value);
	for (int i = 0; i < 4; i++) {
		if (Bytes[i] != 0) {
			write_uint8_hex(Terminal, Bytes[i]);
		} else {
			write_char(Terminal, '0');
		}
	}		
}

void terminal_initialize() {
	initialize(&GlobalTerminal);			
}

void terminal_write_string(const char *String) {
	write_string(&GlobalTerminal, String);
}

void terminal_newline() {
	newline(&GlobalTerminal);
}

void terminal_write_uint8_hex(uint8_t Value) {
	write_uint8_hex(&GlobalTerminal, Value);
}
void terminal_write_uint32_hex(uint32_t Value) {
	write_uint32_hex(&GlobalTerminal, Value);
}
