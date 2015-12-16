#include <stddef.h>
#include <stdint.h>

#include "kernel_terminal.h"
#include "kernel_descriptor_table.h"

void isr_handler(registers Registers) {
	terminal_write_string("[Kernel Interrupt]\n");
	terminal_write_string("Interrupt ID: ");
	// uint8_t InterruptID = (uint8_t)Registers.interrupt_id;
	// terminal_write_uint8_hex(InterruptID);
	terminal_write_uint32_hex(Registers.interrupt_id);
	terminal_newline();
	terminal_write_string("Error Code: ");
	terminal_write_uint8_hex(Registers.error_code);
	terminal_newline();
	terminal_newline();
}

// TODO(Torin) Kernal Panic
void kernel_panic(const char *Message) {
	terminal_write_string("KERNEL PANIC: ");
	terminal_write_string(Message);
	terminal_newline();
}

// TODO(Torin) Kernel Assertion
void kernel_assert() {

}

#define kernel_test_interrupt(InterruptID) asm volatile ("int $0x##InterruptID##")

void kernel_main() {
	terminal_initialize();
	terminal_write_string("[TJW-OS V0.0]\n");

	kernel_gdt GDT;
	kernel_idt IDT;
	gdt_initialize(&GDT);
	idt_initialize(&IDT);

	terminal_write_string("GDT Initialized\n");	
	terminal_write_string("IDT Initialized\n");
	terminal_write_string("Kernel Initalized\n");	
	terminal_write_string("Hello Kernel!\n");

	terminal_write_uint8_hex(0);
	terminal_newline();
	terminal_write_uint8_hex(5);
	terminal_newline();
	terminal_write_uint8_hex(16); 
	terminal_newline();
	terminal_write_uint8_hex(32);
	terminal_newline();
	terminal_write_uint8_hex(255);
	terminal_write_string("\n\n\n");

	terminal_write_uint32_hex(11);
	terminal_newline();
	terminal_write_uint32_hex(4096);

	asm volatile("int $0x02");	
	asm volatile("int $0x03");

	// kernel_test_interrupt(1);
//	kernel_test_interrupt(2);
}

int kernel_initalize() {
	return 0;	
}

int kernal_terminate() {
	return 0;
}
