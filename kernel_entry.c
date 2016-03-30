_Pragma("GCC diagnostic ignored \"-Wunused-variable\"");
_Pragma("GCC diagnostic ignored \"-Wunused-function\"");

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define export extern "C"
#define external extern 
#define internal static
#define global_variable static 
#define static_assert(expr,msg) _Static_assert(expr, msg)
#define kernel_assert(expr,msg)

#include "descriptor_table.c"
#include "keyboard_map.h"
#include "utility.h"

extern "C" {

external uint8_t read_port(uint16_t port);
external void write_port(uint16_t port, uint8_t data);

// Descriptor Tables
//========================================================

struct idt_entry_struct {
  uint16_t offset_0_15; 
  uint16_t code_segment_selector; 
  uint16_t null_byte; 
  uint16_t type_attributes;
  uint16_t offset_16_31; 
} __attribute__((packed));

struct gdt_entry_struct {
	uint16_t limit_0_15;
	uint32_t base_0_23 : 24;

	uint8_t access_isCurrentlyAccessed 	: 1;
	uint8_t access_readWrite : 1;
	uint8_t access_directionBit 				: 1;
	uint8_t access_data0_code1: 1;
	uint8_t access_alwaysSetBit    			: 1;
	uint8_t access_privlegeBit     			: 2;
	uint8_t access_presentBit      			: 1;

	uint8_t limit_16_19 : 4;
	uint8_t flags_avaiable : 1; //system usage thing?
	uint8_t flags_nullBit : 1;
	uint8_t flags_size  : 1;
	uint8_t flags_gran  : 1;

  uint8_t base_24_31;
} __attribute__((packed));

#if 0
struct tss_entry_struct {

} __attribute__((packed));
#endif

typedef struct idt_entry_struct IDT_Entry;
typedef struct gdt_entry_struct GDT_Entry;

//NOTE(Torin) This procedure encodes a gdt_entry_struct into a uint64_t
//It assumes that the kernel will be running in flat 32bit protected mode
//the base address is set to zero and the limit is set to 2^32
internal inline uint64_t 
gdt_encode_entry(uint8_t isCodeSegment, uint8_t privlageLevel) {
	//TODO(Torin) This is not a really a kernel assertion because this code should not ever run during kernel initalzation
	//in the final implementation.  The descriptor values will be computed staticly
  kernel_assert(privlageLevel >= 0 && privlageLevel <= 3, "invalid privlage level");

  GDT_Entry entry;
	//entry.limit_0_15 = (uint16_t)(limit & 0xFFFF);
	//entry.base_0_23  = (uint32_t)(base & 0xFFFFFF);
	entry.limit_0_15 = 0xFF;
	entry.limit_16_19 = 0xF;
	entry.base_0_23 = 0;
	entry.base_24_31 = 0;

	entry.access_isCurrentlyAccessed = 0;
	entry.access_readWrite = 1;
	entry.access_directionBit = 0;
	entry.access_data0_code1 = isCodeSegment ? 1 : 0;
	entry.access_alwaysSetBit = 1;
	entry.access_privlegeBit = privlageLevel;
	entry.access_presentBit = 1;

	entry.flags_avaiable = 1;
	entry.flags_nullBit = 0;
	entry.flags_size = 1;
	entry.flags_gran = 1;
	
	uint64_t result = *((uint64_t *)(&entry));
	return result;
}


#define IDT_ENTRY_COUNT 256
global_variable IDT_Entry idt_entries[IDT_ENTRY_COUNT];
global_variable uint64_t gdt[5];

typedef enum {
  VGAColor_BLACK = 0,
  VGAColor_BLUE  = 1,
  VGAColor_GREEN = 2,
  VGAColor_CYAN  = 3,
  VGAColor_RED   = 4,
  VGAColor_MAGENTA = 5,
  VGAColor_BROWN = 6
} VGAColor;

#define PIC1_COMMAND_PORT 0x20
#define PIC2_COMMAND_PORT 0xA0
#define PIC1_DATA_PORT 0x21
#define PIC2_DATA_PORT 0xA1
#define PIC_EOI_CODE 0x20

#define KEYCODE_A 30
#define KEYCODE_BACKSPACE_PRESSED  0x0E
#define KEYCODE_BACKSPACE_RELEASED 0x8E
#define KEYCODE_ENTER_PRESSED 0x1C

#if 1
global_variable bool is_output_buffer_dirty;
global_variable bool is_input_buffer_dirty;
global_variable bool is_command_ready;
global_variable char input_buffer[256];
global_variable char output_buffer[1024*16];
global_variable uint8_t input_buffer_count = 0;
global_variable uint8_t output_buffer_count = 0;

internal void
stdout_write_cstr(const char *str) {
  uint32_t index = 0;
	while (str[index] != 0) {
		output_buffer[output_buffer_count + index] = str[index];
		index++;
	}

	output_buffer[output_buffer_count + index] = 0;
	output_buffer_count += index + 1;
	is_output_buffer_dirty = true;
}


internal void
stdout_write_fmt(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
  uint32_t index = 0;

	const char *buffer_begin = &output_buffer[output_buffer_count];
	char *write = &output_buffer[output_buffer_count];
	while (fmt[index] != 0) {
		if (fmt[index] == '%') {
			index++;
			if (fmt[index] == 'u') {
				uint32_t value = va_arg(args, uint32_t);
				uint8_t byte3 = (uint8_t)value;
				uint8_t byte2 = (uint8_t)(value >> 8);
				uint8_t byte1 = (uint8_t)(value >> 16);
				uint8_t byte0 = (uint8_t)(value >> 24);
				
				static auto write_byte = [](char *dest, uint8_t value) {
					static auto write_four_bits = [](char *dest, uint8_t four_bits) { 
						if (four_bits > 10) {
							*dest = 'A' + (four_bits - 10);
						} else {
							*dest = '0' + four_bits;
						}
					};

					uint8_t top_4_bits = (uint8_t)(value >> 4);
					uint8_t bottom_4_bits = (uint8_t)((value << 4) >> 4);
					write_four_bits(&dest[0], top_4_bits);
					write_four_bits(&dest[1], bottom_4_bits);
				};


				//memcpy_literal_and_increment(write, "0x");
				write_byte(write + 0, byte3);
				write_byte(write + 2, byte2);
				write_byte(write + 4, byte1);
				write_byte(write + 6, byte0);
				write += 8;
				index++;
			}
		}
		else {
			*write = fmt[index];
			write++;
			index++;
		}
	}

	*write = 0;
	write++;

	size_t bytes_written = write - buffer_begin;
	output_buffer_count += bytes_written;
	is_output_buffer_dirty = true;
}

#define klog(...) stdout_write_fmt(__VA_ARGS__);
#define kdebug(...) stdout_write_fmt(__VA_ARGS__);

#endif

internal inline
void sti() {
	asm volatile ("sti");
}

internal inline
void lidt(void *base, uint16_t size) {
	struct {
		uint16_t size;
		void *base;
	} __attribute__((packed)) idtr = { size, base };
	asm volatile ("lidt %0" : : "m"(idtr));
}

internal inline
void lgdt(void *base, uint16_t size) {
	struct {
    uint16_t size;
		void *base;
	} __attribute__((packed)) gdtr = { size, base };
  asm volatile ("lgdt %0" : : "m"(gdtr));
}


internal void
kernel_reboot() {
	lidt(0, 0);
	asm volatile ("int $0x3");
}

internal void kterm_redraw_if_required();
#include "interrupt_handler.c"

#define LITERAL_STRLEN(literal) (sizeof(literal) - 1)
#define string_matches_literal(string, len, lit) strings_match(string,len, lit, LITERAL_STRLEN(lit))

internal int 
strings_match(const char *stringA, size_t lengthA, 
		const char *stringB, size_t lengthB) {
	if (lengthA != lengthB) return 0;
	for (size_t i = 0; i < lengthA; i++) {
		if (stringA[i] != stringB[i]) {
			return 0;
		}
	}
	return 1;
}


#if 1
internal void
kterm_redraw_if_required() {
	static const uint8_t VGA_TEXT_COLUMN_COUNT = 80;
	static const uint8_t VGA_TEXT_ROW_COUNT = 25;
	static uint8_t *VGA_TEXT_BUFFER = (uint8_t*)(0xB8000);

	//TODO(Torin) This does not belong here
	if (is_command_ready) {
		if (string_matches_literal(input_buffer, input_buffer_count, "reboot")) {
			kernel_reboot();
		} else {
			klog("unknown command");
			memset((uint8_t *)input_buffer, 0, sizeof(input_buffer));
			input_buffer_count = 0;
			is_input_buffer_dirty = true;
		}
		is_command_ready = false;
	}

  if (is_output_buffer_dirty) {
		uint8_t current_color = VGAColor_GREEN;
		uint8_t current_row = 0, current_column = 0;
		
		const char *read = output_buffer;
		while (*read != 0) {
			size_t index = ((current_row * VGA_TEXT_COLUMN_COUNT) + current_column) * 2;
			VGA_TEXT_BUFFER[index] = *read;
			VGA_TEXT_BUFFER[index+1] = current_color;
			current_column++;
			if (current_column > VGA_TEXT_COLUMN_COUNT) {
				current_column = 2;
				current_row++;
			}
			read++;
			if (*read == 0) {
				read++;
				current_row++;
				current_column = 0;
			} 
		}
		is_output_buffer_dirty = false;
	}


	if (is_input_buffer_dirty) {
		uint32_t current_row = VGA_TEXT_ROW_COUNT - 1;
		uint32_t current_column = 0;
		uint32_t index = ((current_row * VGA_TEXT_COLUMN_COUNT) + current_column) * 2;

		VGA_TEXT_BUFFER[index] = '>';
		VGA_TEXT_BUFFER[index+1] = VGAColor_GREEN;
		index += 2;
		current_column += 1;
	
		for (uint32_t i = 0; i < input_buffer_count; i++) {
		  VGA_TEXT_BUFFER[index] = input_buffer[i];
			VGA_TEXT_BUFFER[index+1] = VGAColor_GREEN;
			index += 2;
			current_column++;
		}

		uint32_t diff = VGA_TEXT_COLUMN_COUNT - current_column;
		for (uint32_t i = 0; i < diff; i+=2) {
			VGA_TEXT_BUFFER[index] = 0;
			VGA_TEXT_BUFFER[index+1] = 0;
			index += 2;
		}

		is_input_buffer_dirty = false;
	}

}

#endif


internal inline
void idt_install_interrupt(const uint32_t irq_number, const uint32_t irq_handler_addr) {
  static const uint8_t INTERRUPT_GATE_32   = 0x8E;
	static const uint8_t GDT_CODE_SEGMENT_OFFSET = 0x08;

  idt_entries[irq_number].offset_0_15 = irq_handler_addr & 0xFFFF; 
  idt_entries[irq_number].code_segment_selector = GDT_CODE_SEGMENT_OFFSET;
  idt_entries[irq_number].null_byte = 0;
  idt_entries[irq_number].type_attributes = INTERRUPT_GATE_32; 
  idt_entries[irq_number].offset_16_31 = ((irq_handler_addr & 0xFFFF0000) >> 16);
}

//TODO(Torin) Replace this with the apic

//====================================================================

internal void
x86_pic8259_initalize(void)
{ //@Initalize And Remap the @8259_PIC @PIC @pic
	static const uint32_t ICW1_INIT_CASCADED = 0x11;
	static const uint32_t ICW2_PIC1_IRQ_NUMBER_BEGIN = 0x20;
	static const uint32_t ICW2_PIC2_IRQ_NUMBER_BEGIN = 0x28;
	static const uint32_t ICW3_PIC1_IRQ_LINE_2 = 0x4;
	static const uint32_t ICW3_PIC2_IRQ_LINE_2 = 0x2;
	static const uint32_t ICW4_8068 = 0x01;

	//Initalization Command Words (ICW)
	//ICW1 Tells PIC to wait for 3 more words
	write_port(PIC1_COMMAND_PORT, ICW1_INIT_CASCADED);
	write_port(PIC2_COMMAND_PORT, ICW1_INIT_CASCADED);
	//ICW2 Set PIC Offset Values
	write_port(PIC1_DATA_PORT, ICW2_PIC1_IRQ_NUMBER_BEGIN);
	write_port(PIC2_DATA_PORT, ICW2_PIC2_IRQ_NUMBER_BEGIN);
	//ICW3 PIC Cascading Info
	write_port(PIC1_DATA_PORT, ICW3_PIC1_IRQ_LINE_2);
	write_port(PIC2_DATA_PORT, ICW3_PIC2_IRQ_LINE_2);
	//ICW4 Additional Enviroment Info
	//NOTE(Torin) Currently set to 80x86
	write_port(PIC1_DATA_PORT, ICW4_8068);
	write_port(PIC2_DATA_PORT, ICW4_8068);
	klog("PIC8259 Initialized and Remaped");
}

internal void
x86_pit_initialize(void)  
{
	static const uint32_t FREQUENCY = 20; //20Hz = 20ms

	static const uint32_t PIT_DATA_PORT0 = 0x40;
	static const uint32_t PIT_DATA_PORT1 = 0x41;
	static const uint32_t PIT_DATA_PORT2 = 0x42;
	static const uint32_t PIT_COMMAND_PORT = 0x43;
	static const uint32_t PIT_INTERNAL_FREQUENCY = 1193180;
	static const uint32_t PIT_COMMAND_REPEATING_MODE = 0x36;

	static const uint32_t PIT_DIVISOR = PIT_INTERNAL_FREQUENCY / FREQUENCY;
	uint8_t divisor_low = (uint8_t)(PIT_DIVISOR & 0xFF);
	uint8_t divisor_high = (uint8_t)((PIT_DIVISOR >> 8) & 0xFF);

	write_port(PIT_COMMAND_PORT, PIT_COMMAND_REPEATING_MODE);
	write_port(PIT_DATA_PORT0, divisor_low);
	write_port(PIT_DATA_PORT0, divisor_high);
	klog ("PIT initialized!");
}

//===========================================================================

internal void
x86_gdt_initialize(void) 
{
	//TODO(Torin) Why is the limit address so tiny here?
	//It should probably be set to 0xFFFFFFFF
	//to address the full 32bit address space
	gdt[0] = gdt_create_descriptor(0, 0, 0);
	gdt[1] = gdt_create_descriptor(0, 0x000FFFFF, (GDT32_CODE_PL0));
	gdt[2] = gdt_create_descriptor(0, 0x000FFFFF, (GDT32_DATA_PL0));
	gdt[3] = gdt_create_descriptor(0, 0x000FFFFF, (GDT32_CODE_PL3));
	gdt[4] = gdt_create_descriptor(0, 0x000FFFFF, (GDT32_DATA_PL3));
	lgdt(gdt, sizeof(gdt));
	klog("GDT Inititalized");
}

internal void
x86_idt_initalize() {

	//Clear and initialize the idt to an unhandled stub for debugging
	memset((uint8_t*)idt_entries, 0, sizeof(idt_entries));
	extern void asm_irq_unhandled_stub(void);
	for (uint32_t i = 0; i < 256; i++) {
		idt_install_interrupt(i, (uint32_t)asm_irq_unhandled_stub);
	}
	
	{ //Software Exceptions
		static const uint32_t IRQ_DIVDE_BY_ZERO = 0;
		static const uint32_t IRQ_DEBUG_EXCEPTION = 1;
		static const uint32_t IRQ_NON_MASKABLE_INTERRUPT = 2;
		static const uint32_t IRQ_BREAKPOINT_EXCEPTION = 3;

		external void asm_irq_debug(void);
		external void asm_irq_breakpoint(void);

		//idt_install_interrupt(IRQ_DEBUG_EXCEPTION, (size_t)asm_irq_debug);
		//idt_install_interrupt(IRQ_BREAKPOINT_EXCEPTION, (size_t)asm_irq_breakpoint);
	}

	{ //Hardware Interrupts
		static const uint32_t IRQ_PIT = 0x20; 
		static const uint32_t IRQ_KEYBOARD = 0x21;
	
		extern void asm_irq0(void);
		extern void asm_irq1(void);
		extern void asm_irq_handler_keyboard(void);

		interrupt_handlers[0] = irq_handler_pit;
		interrupt_handlers[1] = irq_handler_keyboard;
		idt_install_interrupt(IRQ_PIT, (size_t)asm_irq0);
		idt_install_interrupt(IRQ_KEYBOARD, (size_t)asm_irq1);
	}

	lidt(idt_entries, sizeof(IDT_Entry) * IDT_ENTRY_COUNT);
	write_port(PIC1_DATA_PORT, 0b11111101);
	write_port(PIC2_DATA_PORT, 0b11111111);
	klog("IDT initialized");
	sti();
}


internal inline
size_t strlen(const char *str) {
	size_t result = 0;
	while (str[result] != 0)
		result ++;
	return result;
}

internal void kterm_clear_screen() {
	static const uint8_t VGA_TEXT_COLUMN_COUNT = 80;
	static const uint8_t VGA_TEXT_ROW_COUNT = 25;
	uint8_t *VGA_TEXT_BUFFER = (uint8_t*)(0xB8000);
	memset(VGA_TEXT_BUFFER, 0, 
			(VGA_TEXT_COLUMN_COUNT * VGA_TEXT_ROW_COUNT) * 2);
}

export
void kernel_entry(void) {
	kterm_clear_screen();
	x86_gdt_initialize();
	x86_pic8259_initalize();
	x86_idt_initalize();
	x86_pit_initialize();

  klog("[TwiebsOS] v0.4 Initialized");
	kterm_redraw_if_required();

  for(;;) 
	{
		kterm_redraw_if_required();
	}
}

} //extern "C" garbage 
