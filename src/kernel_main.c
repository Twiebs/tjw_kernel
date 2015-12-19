#include "kernel_types.h"
#include "kernel_terminal.h"
#include "kernel_descriptor_table.h"
#include "kernel_asm.h"
#include "kernel_pic.h"
#include "kernel_debug.h"

#if 0 
static const char* SOFTWARE_INTERRUPT_NAMES[255] = {
	"Divide By Zero",
	"Single Step Trap",

};
#endif

void isr_handler(registers Registers) {
	terminal_write_string("[Kernel Interrupt]\n");
	terminal_write_string("Interrupt ID: ");
	if (Registers.interrupt_id == 0) {
		terminal_write_string("Divide by zero fault");
	}

	if (Registers.interrupt_id == 2) {
		terminal_write_string("NMI Trap");
	}

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

extern void isr32();

// Let just install a PIC interrupt handle so that we can confirm that this is operating as expected!
//  :w
//  a
//
//
//
//

 
void kernel_main() {
	terminal_initialize();
	terminal_write_string("[TJW-OS V0.0]\n");	

	kernel_gdt GDT;
	kernel_idt IDT;
	gdt_initialize(&GDT);
	terminal_write_string("GDT Initialized\n");	
	idt_initialize(&IDT);
	terminal_write_string("IDT Initialized\n");

	// 32 - 40 	
	x86_pic_initialize(0x20, 0x28);
	debug_play_speaker(6000);

//	idt_set_entry(&IDT, 32, );

	idt_set_entry(&IDT, 32, (uint32_t)isr32, 0x08, 0x8E);

	terminal_write_string("PIC Remapped!\n");

#if 0
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
#endif

		

	asm volatile("int $0x02");	
	asm volatile("int $0x03");
}

int kernel_initalize() {
	return 0;	
}

int kernal_terminate() {
	return 0;
}
