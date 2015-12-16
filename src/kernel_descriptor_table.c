#include "kernel_descriptor_table.h"

#include <stddef.h>

extern void idt_flush();
extern void gdt_flush();

extern void isr0();
extern void isr1();
extern void isr2();
 
static void gdt_set_gate(kernel_gdt *GDT, int32_t Index, uint32_t Base, 
						uint32_t Limit, uint8_t Access, uint8_t Gran) {
	GDT->Entries[Index].base_low = (Base & 0xFFFF);
	GDT->Entries[Index].base_mid = (Base >> 16) & 0xFF;
	GDT->Entries[Index].base_high = (Base >> 24) & 0xFF;
	GDT->Entries[Index].limit_low = (Limit & 0xFFFF);
	GDT->Entries[Index].granularity = (Limit >> 16) & 0x0F;
	GDT->Entries[Index].granularity |= Gran & 0xF0;
	GDT->Entries[Index].access = Access;
}


static void idt_set_gate(kernel_idt *IDT, uint8_t Index, uint32_t Base, uint16_t Selector, uint8_t Flags) {
	IDT->Entries[Index].BaseLow = (Base & 0xFFFF);
	IDT->Entries[Index].BaseHigh = (Base >> 16) & 0xFFFF;
	IDT->Entries[Index].Selector = Selector;
	IDT->Entries[Index].NullByte = 0;
	IDT->Entries[Index].Flags = Flags;
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


static inline void mem_set(void *Buffer, uint8_t Value, size_t Count) {
	uint8_t* Data = (uint8_t*)Buffer;	
	for (size_t i = 0; i < Count; i++) {
		Data[i] = Value;
	}
}

void idt_initialize(kernel_idt *IDT) {
	IDT->Ptr.Limit = (sizeof(kernel_idt_entry) * 256) - 1;
	IDT->Ptr.Base = (uint32_t)&IDT->Entries;
	mem_set(&IDT->Entries, 0, sizeof(kernel_idt_entry) * 256);

	idt_set_gate(IDT, 0, (uint32_t)isr0, 0x08, 0x8E);
	idt_set_gate(IDT, 1, (uint32_t)isr1, 0x08, 0x8E);
	idt_set_gate(IDT, 2, (uint32_t)isr2, 0x08, 0x8E);
	idt_flush();
}


