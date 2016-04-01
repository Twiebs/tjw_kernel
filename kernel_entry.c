_Pragma("GCC diagnostic ignored \"-Wunused-variable\"");
_Pragma("GCC diagnostic ignored \"-Wunused-function\"");

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define export extern "C"
#define external extern "C"
#define internal static
#define global_variable static 

#define kassert(expr, msg) stdout_write_fmt(msg);
#define kerror(...) stdout_write_fmt(__VA_ARGS__);
#define klog(...) stdout_write_fmt(__VA_ARGS__);
#define kdebug(...) stdout_write_fmt(__VA_ARGS__);

internal void
stdout_write_fmt(const char *fmt, ...);

extern "C" {
#include "utility.h"
#include "descriptor_table.c"
#include "kernel_memory.cpp"
#include "keyboard_map.h"
#include "kernel_feature.c"
}

external uint8_t read_port(uint16_t port);
external void write_port(uint16_t port, uint8_t data);

// Descriptor Tables
//========================================================

struct idt_entry_struct {
  uint16_t offset_0_15; 
  uint16_t code_segment_selector; 
  uint8_t null_byte; 
  uint8_t type_attributes;
  uint16_t offset_16_31; 
} __attribute__((packed));

struct gdt_entry_struct {
	uint16_t limit_0_15;
	uint16_t base_0_15;
	uint8_t base_16_23;

#if 0
	uint8_t access_isCurrentlyAccessed 	: 1;
	uint8_t access_readWrite 						: 1;
	uint8_t access_directionBit 				: 1;
	uint8_t access_data0_code1					: 1;
	uint8_t access_alwaysSetBit    			: 1;
	uint8_t access_privlegeBit     			: 2;
	uint8_t access_presentBit      			: 1;
#endif

	uint8_t access;
	uint8_t granularity;

  uint8_t base_24_31;
} __attribute__((packed));

static_assert(sizeof(idt_entry_struct) == 8, "invalid idt_entry_struct");
static_assert(sizeof(gdt_entry_struct) == 8, "invalid gdt_entry_struct");

#define IDT_ENTRY_COUNT 256

global_variable idt_entry_struct idt_entries[IDT_ENTRY_COUNT];
global_variable gdt_entry_struct _gdt[3]; 

typedef enum {
  VGAColor_BLACK = 0,
  VGAColor_BLUE  = 1,
  VGAColor_GREEN = 2,
  VGAColor_CYAN  = 3,
  VGAColor_RED   = 4,
  VGAColor_MAGENTA = 5,
  VGAColor_BROWN = 6
} VGAColor;

#define KEYCODE_A 30
#define KEYCODE_BACKSPACE_PRESSED  0x0E
#define KEYCODE_BACKSPACE_RELEASED 0x8E
#define KEYCODE_ENTER_PRESSED 0x1C

global_variable bool is_output_buffer_dirty;
global_variable bool is_input_buffer_dirty;
global_variable bool is_command_ready;
global_variable char input_buffer[256];
global_variable char output_buffer[1024*16];
global_variable uint32_t input_buffer_count = 0;
global_variable uint32_t output_buffer_count = 0;

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

				uint8_t byte3 = (uint8_t)(value >> 24);
				uint8_t byte2 = (uint8_t)(value >> 16);
				uint8_t byte1 = (uint8_t)(value >> 8);
				uint8_t byte0 = (uint8_t)value;
				
				static auto write_byte = [](char *dest, uint8_t value) {
					static auto write_four_bits = [](char *dest, uint8_t four_bits) { 
						if (four_bits > 9) {
							*dest = 'A' + (four_bits - 10);
						} else {
							*dest = '0' + four_bits;
						}
					};

					uint8_t top_4_bits = (uint8_t)(value >> 4);
					uint8_t bottom_4_bits = (uint8_t)((uint8_t)(value << 4) >> 4);
					write_four_bits(&dest[0], top_4_bits);
					write_four_bits(&dest[1], bottom_4_bits);
				};

				memcpy_literal_and_increment(write, "0x");
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
void idt_install_interrupt(const uint32_t irq_number, const uintptr_t irq_handler_addr) {
  static const uint8_t INTERRUPT_GATE_32 = 0x8E;
	static const uint8_t GDT_CODE_SEGMENT_OFFSET = 0x08;

  idt_entries[irq_number].offset_0_15 = (uint16_t)(irq_handler_addr & 0xFFFF);
  idt_entries[irq_number].offset_16_31 = (uint16_t)((irq_handler_addr >> 16) & 0xFFFF);
  idt_entries[irq_number].code_segment_selector = GDT_CODE_SEGMENT_OFFSET;
  idt_entries[irq_number].null_byte = 0;
  idt_entries[irq_number].type_attributes = INTERRUPT_GATE_32; 
}

//TODO(Torin) Replace this with the apic
//====================================================================

internal void
x86_pic8259_initalize(void)
{ //@Initalize And Remap the @8259_PIC @PIC @pic
	static const uint8_t PIC1_COMMAND_PORT = 0x20;
	static const uint8_t PIC2_COMMAND_PORT = 0xA0;
	static const uint8_t PIC1_DATA_PORT = 0x21;
	static const uint8_t PIC2_DATA_PORT = 0xA1;

	static const uint8_t ICW1_INIT_CASCADED = 0x11;
	static const uint8_t ICW2_PIC1_IRQ_NUMBER_BEGIN = 0x20;
	static const uint8_t ICW2_PIC2_IRQ_NUMBER_BEGIN = 0x28;
	static const uint8_t ICW3_PIC1_IRQ_LINE_2 = 0x4;
	static const uint8_t ICW3_PIC2_IRQ_LINE_2 = 0x2;
	static const uint8_t ICW4_8068 = 0x01;

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

	write_port(PIC1_DATA_PORT, 0b11111100);
	write_port(PIC2_DATA_PORT, 0b11111111);
	klog("PIC8259 Initialized and Remaped");
}

internal void
x86_pit_initialize(void)  
{
	static const uint32_t FREQUENCY = 100; 
	
	static const uint32_t PIT_INTERNAL_FREQUENCY = 1193180;
	static const uint32_t PIT_COMMAND_REPEATING_MODE = 0x36;
	static const uint32_t PIT_DIVISOR = PIT_INTERNAL_FREQUENCY / FREQUENCY;

	static const uint8_t PIT_DATA_PORT0 = 0x40;
	static const uint8_t PIT_DATA_PORT1 = 0x41;
	static const uint8_t PIT_DATA_PORT2 = 0x42;
	static const uint8_t PIT_COMMAND_PORT = 0x43;

	uint8_t divisor_low = (uint8_t)(PIT_DIVISOR & 0xFF);
	uint8_t divisor_high = (uint8_t)((PIT_DIVISOR >> 8) & 0xFF);

	write_port(PIT_COMMAND_PORT, PIT_COMMAND_REPEATING_MODE);
	write_port(PIT_DATA_PORT0, divisor_low);
	write_port(PIT_DATA_PORT0, divisor_high);
	klog ("PIT initialized!");
}

//===========================================================================
//

internal void
gdt_set_entry(uint32_t n, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	_gdt[n].base_0_15 = (base & 0xFFFF);
	_gdt[n].base_16_23 = (base >> 16) && 0xFF;
	_gdt[n].base_24_31 = (base >> 24) && 0xFF;

	_gdt[n].limit_0_15 = (limit & 0xFFFF);
	_gdt[n].granularity = (limit >> 16) & 0x0F;
	
	_gdt[n].granularity |= gran & 0xF0;
	_gdt[n].access = access;
}

external void enter_protected_mode();

internal void
x86_gdt_initialize(void) 
{
	gdt_set_entry(0, 0, 0, 0, 0);
	gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
	lgdt(_gdt, sizeof(_gdt));
	enter_protected_mode();
	klog("GDT Inititalized");
}

extern "C" {

internal void
x86_idt_initalize() {
	//Clear and initialize the idt to an unhandled stub for debugging
	memset(idt_entries, 0, sizeof(idt_entries));
	extern void asm_irq_unhandled_stub(void);
	for (uint32_t i = 0; i < 256; i++) {
		idt_install_interrupt(i, (uintptr_t)asm_irq_unhandled_stub);
	}
	
	{ //Software Exceptions
		extern void asm_isr0(void);
		extern void asm_isr1(void);
		extern void asm_isr2(void);
		extern void asm_isr3(void);
		extern void asm_isr4(void);
		extern void asm_isr5(void);
		extern void asm_isr6(void);
		extern void asm_isr7(void);
		extern void asm_isr8(void);
		extern void asm_isr9(void);
		extern void asm_isr10(void);
		extern void asm_isr11(void);
		extern void asm_isr12(void);
		extern void asm_isr13(void);
		extern void asm_isr14(void);
		extern void asm_isr15(void);
		extern void asm_isr16(void);
		extern void asm_isr17(void);
		extern void asm_isr18(void);
		extern void asm_isr19(void);
		extern void asm_isr20(void);
		extern void asm_isr21(void);
		extern void asm_isr22(void);
		extern void asm_isr23(void);
		extern void asm_isr24(void);
		extern void asm_isr25(void);
		extern void asm_isr26(void);
		extern void asm_isr27(void);
		extern void asm_isr28(void);
		extern void asm_isr29(void);
		extern void asm_isr30(void);
		extern void asm_isr31(void);
		extern void asm_isr32(void);

		idt_install_interrupt(0, (uintptr_t)asm_isr0);
		idt_install_interrupt(1, (uintptr_t)asm_isr1);
		idt_install_interrupt(2, (uintptr_t)asm_isr2);
		idt_install_interrupt(3, (uintptr_t)asm_isr3);
		idt_install_interrupt(4, (uintptr_t)asm_isr4);
		idt_install_interrupt(5, (uintptr_t)asm_isr5);
		idt_install_interrupt(6, (uintptr_t)asm_isr6);
		idt_install_interrupt(7, (uintptr_t)asm_isr7);
		idt_install_interrupt(8, (uintptr_t)asm_isr8);
		idt_install_interrupt(9, (uintptr_t)asm_isr9);
		idt_install_interrupt(10, (uintptr_t)asm_isr10);
		idt_install_interrupt(11, (uintptr_t)asm_isr11);
		idt_install_interrupt(12, (uintptr_t)asm_isr12);
		idt_install_interrupt(13, (uintptr_t)asm_isr13);
		idt_install_interrupt(14, (uintptr_t)asm_isr14);
		idt_install_interrupt(15, (uintptr_t)asm_isr15);
		idt_install_interrupt(16, (uintptr_t)asm_isr16);
		idt_install_interrupt(17, (uintptr_t)asm_isr17);
		idt_install_interrupt(18, (uintptr_t)asm_isr18);
		idt_install_interrupt(19, (uintptr_t)asm_isr19);
		idt_install_interrupt(20, (uintptr_t)asm_isr20);
		idt_install_interrupt(21, (uintptr_t)asm_isr21);
		idt_install_interrupt(22, (uintptr_t)asm_isr22);
		idt_install_interrupt(23, (uintptr_t)asm_isr23);
		idt_install_interrupt(24, (uintptr_t)asm_isr24);
		idt_install_interrupt(25, (uintptr_t)asm_isr25);
		idt_install_interrupt(26, (uintptr_t)asm_isr26);
		idt_install_interrupt(27, (uintptr_t)asm_isr27);
		idt_install_interrupt(28, (uintptr_t)asm_isr28);
		idt_install_interrupt(29, (uintptr_t)asm_isr29);
		idt_install_interrupt(30, (uintptr_t)asm_isr30);
		idt_install_interrupt(31, (uintptr_t)asm_isr31);
		idt_install_interrupt(32, (uintptr_t)asm_isr32);
	}

	{ //Hardware Interrupts
		static const uint32_t IRQ_PIT = 0x20; 
		static const uint32_t IRQ_KEYBOARD = 0x21;
	
		extern void asm_irq0(void);
		extern void asm_irq1(void);

		interrupt_handlers[0] = irq_handler_pit;
		interrupt_handlers[1] = irq_handler_keyboard;
		idt_install_interrupt(IRQ_PIT, (uintptr_t)asm_irq0);
		idt_install_interrupt(IRQ_KEYBOARD, (uintptr_t)asm_irq1);
	}

	lidt(idt_entries, sizeof(idt_entries));
  klog("IDT initialized");
	sti();
}

}

internal void kterm_clear_screen() {
	static const uint8_t VGA_TEXT_COLUMN_COUNT = 80;
	static const uint8_t VGA_TEXT_ROW_COUNT = 25;
	uint8_t *VGA_TEXT_BUFFER = (uint8_t*)(0xB8000);
	memset(VGA_TEXT_BUFFER, 0, 
			(VGA_TEXT_COLUMN_COUNT * VGA_TEXT_ROW_COUNT) * 2);
}

external const uint32_t kernelend;

internal inline
void safemode_checks() {

#define support_check(x,func) \
	if (func()) { klog(x " : SUPPORTED"); } \
	else { klog(x " : UNSUPPORTED"); } \

	klog("[System Info]");

	support_check("cpuid", cpuid_is_supported);
	support_check("apic", cpuid_is_apic_supported);
	support_check("longmode", cpuid_is_longmode_supported);
}

export
void kernel_entry(void) {
	kterm_clear_screen();

	safemode_checks();
	
	x86_gdt_initialize();
	x86_pic8259_initalize();
	x86_idt_initalize();
	x86_pit_initialize();

  klog("[TwiebsOS] v0.0 Initialized");

	//kmem_initialize();

#if 0

	uint32_t *ptr = (uint32_t *)0xFFFFFFFF;
	*ptr = 7;

	uint32_t *ptrB = (uint32_t *)0;
	*ptrB = 6;

	uint32_t *ptrD = (uint32_t *)(0x1000 * 3);
	*ptrD = 200;
	uint32_t fault_me = *ptrD;

	uint32_t *this_will_work = (uint32_t *)(&_first_page_table + sizeof(PageTable) + 4);
	*this_will_work = 100;

	uint32_t *ptrC = (uint32_t *)0xA0000000;
	uint32_t page_fault = *ptrC;
#endif

	while (1) { asm volatile("hlt"); };
}

