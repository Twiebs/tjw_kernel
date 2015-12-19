#include "kernel_asm.h"
#include "kernel_descriptor_table.h"

struct idtr_struct {
	uint16_t Limit;
	uint32_t Base;
} __attribute__((packed));

struct gdtr_struct {
	uint16_t Limit;
	uint32_t Base;
} __attribute__((packed));

typedef struct idtr_struct idtr_t;
typedef struct gdtr_struct gdtr_t;

// External ASM Functions
extern void gdt_install_asm(uint32_t gdtr_addr);
extern void idt_install_asm(uint32_t idtr_addr);

static void gdt_set_entry(kernel_gdt *GDT, int32_t Index, uint32_t Base, 
						uint32_t Limit, uint8_t Access, uint8_t Gran) {
	GDT->Entries[Index].base_low = (Base & 0xFFFF);
	GDT->Entries[Index].base_mid = (Base >> 16) & 0xFF;
	GDT->Entries[Index].base_high = (Base >> 24) & 0xFF;
	GDT->Entries[Index].limit_low = (Limit & 0xFFFF);
	GDT->Entries[Index].granularity = (Limit >> 16) & 0x0F;
	GDT->Entries[Index].granularity |= Gran & 0xF0;
	GDT->Entries[Index].access = Access;
}

void idt_set_entry(kernel_idt *IDT, uint8_t Index, uint32_t Base, uint16_t Selector, uint8_t Flags) {
	IDT->Entries[Index].BaseLow = (Base & 0xFFFF);
	IDT->Entries[Index].BaseHigh = (Base >> 16) & 0xFFFF;
	IDT->Entries[Index].Selector = Selector;
	IDT->Entries[Index].NullByte = 0;
	IDT->Entries[Index].Flags = Flags;
}

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

void gdt_initialize(kernel_gdt *GDT) {
	gdt_set_entry(GDT, 0, 0, 0, 0, 0); 				 // Null segment
	gdt_set_entry(GDT, 1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
	gdt_set_entry(GDT, 2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
	gdt_set_entry(GDT, 3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
	gdt_set_entry(GDT, 4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

	gdtr_t GDTR;
	GDTR.Limit = (sizeof(kernel_gdt_entry) * 5) - 1;
	GDTR.Base = (uint32_t)&GDT->Entries;
	gdt_install_asm((uint32_t)&GDTR);
}


static inline void mem_set(void *Buffer, uint8_t Value, size_t Count) {
	uint8_t* Data = (uint8_t*)Buffer;	
	for (size_t i = 0; i < Count; i++) {
		Data[i] = Value;
	}
}

#include "kernel_terminal.h"
static void ir_default_handler() {
	terminal_write_string("DEFUALT IR HANDLER WAS TRIGGERED \n\n\n\n\n\n\n\n");
	terminal_write_string("The Kernel is now hanging!");
	for(;;);
}


// NOTE(Torin) 0x8 is the code segment.  Should it by abstracted?
void idt_initialize(kernel_idt *IDT) {
	mem_set(IDT->Entries, 0, sizeof(kernel_idt_entry) * 256);
	for (int i = 0; i < 256; i++) {
		idt_set_entry(IDT, i, IDT_DESC_PRESENT | IDT_DESC_BIT32,
			0x8, (uint32_t)ir_default_handler);
	}

	idtr_t IDTR;
	IDTR.Limit = (sizeof(kernel_idt_entry) * 256) - 1;
	IDTR.Base = (uint32_t)&IDT->Entries;
	idt_install_asm((uint32_t)&IDTR);
}


