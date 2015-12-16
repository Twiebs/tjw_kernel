#include <stddef.h>
#include <stdint.h>

#include "kernel_terminal.h"
#include "kernel_descriptor_table.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void isr_handler(registers Registers) {
	terminal_write_string("Recieved Interrupt");

}
#pragma GCC diagnostic pop

// TODO(Torin) Kernal Panic
void kernel_panic(const char *Message) {
	terminal_write_string("KERNEL PANIC: ");
	terminal_write_string(Message);
	terminal_newline();
}

// TODO(Torin) Kernel Assertion
void kernel_assert() {

}

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

	asm volatile ("int $0x2");
}

int kernel_initalize() {
	return 0;	
}

int kernal_terminate() {
	return 0;
}
