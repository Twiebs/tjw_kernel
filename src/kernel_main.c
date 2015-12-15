#include <stddef.h>
#include <stdint.h>

#include "kernel_vga.h"
#include "kernel_terminal.h"

struct kernel_gdt_entry_struct {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed));
typedef struct kernel_gdt_entry_struct kernel_gdt_entry;

// TODO(Torin) These ptr structs are pretty useless
// Either combine the IDT and GDT Ptr struct into a 
// DescriptorPtrStruct or just implement these values the kernel_gdt
// and kernel_idt structs respectivly
struct kernel_gdt_ptr_struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));
typedef struct kernel_gdt_ptr_struct kernel_gdt_ptr; 

typedef struct {
	kernel_gdt_ptr GDTPtr;
	kernel_gdt_entry Entries[5];
} kernel_gdt;

struct kernel_idt_entry_struct {
	uint16_t BaseLow;
	uint16_t Selector;
	uint8_t NullByte;
	uint8_t Flags;
	uint16_t BaseHigh;
} __attribute__((packed));
typedef struct kernel_idt_entry_struct kernel_idt_entry;

struct kernel_idt_ptr_struct {
	uint16_t Limit;
	uint32_t Base;
} __attribute__((packed));
typedef struct kernel_idt_ptr;

typedef struct {
	kernel_idt_ptr Ptr;
	kernel_idt_entry Entries[256];
} kernel_idt;

static void mem_set(void *Dest, uint8_t Value, size_t Count) {
	for (size_t i = 0; i < Count; i++) {
		Dest[i] = Value;
	}
}

extern void gdt_flush();

static void gdt_set_gate(kernel_gdt *GDT, int32_t Index, uint32_t Base, 
						uint32_t Limit, uint8_t Access, uint8_t Gran) 
{
	GDT->Entries[Index].base_low = (Base & 0xFFFF);
	GDT->Entries[Index].base_mid = (Base >> 16) & 0xFF;
	GDT->Entries[Index].base_high = (Base >> 24) & 0xFF;
	GDT->Entries[Index].limit_low = (Limit & 0xFFFF);
	GDT->Entries[Index].granularity = (Limit >> 16) & 0x0F;
	GDT->Entries[Index].granularity |= Gran & 0xF0;
	GDT->Entries[Index].access = Access;
}


void gdt_initialize(kernel_gdt *GDT) {
	GDT->GDTPtr.limit = (sizeof(kernel_gdt_entry) * 5) - 1;
	GDT->GDTPtr.base = (uint32_t)&GDT->Entries;
	gdt_set_gate(GDT, 0, 0, 0, 0, 0); 				 // Null segment
	gdt_set_gate(GDT, 1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
	gdt_set_gate(GDT, 2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
	gdt_set_gate(GDT, 3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
	gdt_set_gate(GDT, 4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment
	gdt_flush();
}

void idt_set_gate(kernel_idt *IDT, uint8_t Index, uint32_t Base, uint16_t Selector; uint8_t Flags) {
	IDT->Entries[Index].BaseLow = (Base & 0xFFFF);
	IDT->Entries[Index].BaseHigh = (Base >> 16) & 0xFFFF;
	IDT->Entries[Index].Selector = Selector;
	IDT->Entries[Index].NullByte = 0;
	IDT->Entries[Index].Flags = Flags;
}

void idt_initalize(kernel_idt *IDT) {
	IDT->Ptr->Limit = (sizeof(kernel_idt_entry) * 256) - 1;
	IDT->Ptr->Base = (uint32_t)&IDT->Entries;
	mem_set(&IDT->Entries, 0, sizeof(kernel_idt_entry) * 256);

	idt_set_gate(IDT, 0, (uint32_t)isr0(), 0x08, 0x8E);
	idt_set_gate(IDT, 1, (uint32_t)isr1(), 0x08, 0x8E);


}

// TODO(Torin) Kernel Assertion
void kernel_assert() {

}

void kernel_main() {
	terminal Terminal;	
	terminal_initialize(&Terminal);
	terminal_write_string(&Terminal, "[TJW-OS V0.0]\n");

	kernel_gdt GDT;
	gdt_initialize(&GDT);
	terminal_write_string(&Terminal, "GDT Initialized\n");	

	terminal_write_string(&Terminal, "Kernel Initalized\n");	
	terminal_write_string(&Terminal, "Hello Kernel!\n");
}

int kernel_initalize() {
	return 0;	
}

int kernal_terminate() {
	return 0;
}
