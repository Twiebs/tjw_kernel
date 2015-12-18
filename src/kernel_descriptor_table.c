#include "kernel_descriptor_table.h"
#include "kernel_asm.h"

typedef unsigned size_t;

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

extern void gdt_install(uint32_t gdtr_addr);
extern void idt_install(uint32_t idtr_addr);

#if 0
static const char* ivt_description_strings {
	"Divide by Zero",
	"Single Step (Debugger)",
	"Non Maskable Interrupt (NMI) Pin"
};
#endif

#define PIC_MASTER_COMAND_STATUS 0x20
#define PIC_MASTER_INTERRUPT_MASK 0x21
#define PIC_SLAVE_COMAND_STATUS 0xA0
#define PIC_SLAVE_INTERRUPT_MASK 0xA1

static inline void icw1() {
//	static const uint8_t ICW = 0x11;
	
}

void device_initalize() {
				
}

void device_terminate() {
	
}
#if 0
static void generate_interrupt(int n) {
	asm (
		"mov al, byte ptr[n]"
		"mov byte ptr[genint+1], al"
		"jmp genint"
		"genint:"
			"int 0"
		);	
}
#endif
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
	gdt_set_gate(GDT, 0, 0, 0, 0, 0); 				 // Null segment
	gdt_set_gate(GDT, 1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
	gdt_set_gate(GDT, 2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
	gdt_set_gate(GDT, 3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
	gdt_set_gate(GDT, 4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

	gdtr_t GDTR;
	GDTR.Limit = (sizeof(kernel_gdt_entry) * 5) - 1;
	GDTR.Base = (uint32_t)&GDT->Entries;
	gdt_install((uint32_t)&GDTR);
}


static inline void mem_set(void *Buffer, uint8_t Value, size_t Count) {
	uint8_t* Data = (uint8_t*)Buffer;	
	for (size_t i = 0; i < Count; i++) {
		Data[i] = Value;
	}
}

void idt_initialize(kernel_idt *IDT) {
	mem_set(&IDT->Entries, 0, sizeof(kernel_idt_entry) * 256);
	idt_set_gate(IDT, 0, (uint32_t)isr0, 0x08, 0x8E);
	idt_set_gate(IDT, 1, (uint32_t)isr1, 0x08, 0x8E);
	idt_set_gate(IDT, 2, (uint32_t)isr2, 0x08, 0x8E);
	idt_set_gate(IDT, 3, (uint32_t)isr3, 0x08, 0x8E);
	idt_set_gate(IDT, 4, (uint32_t)isr4, 0x08, 0x8E);

	idtr_t IDTR;
	IDTR.Limit = (sizeof(kernel_idt_entry) * 256) - 1;
	IDTR.Base = (uint32_t)&IDT->Entries;
	idt_install((uint32_t)&IDTR);
}


