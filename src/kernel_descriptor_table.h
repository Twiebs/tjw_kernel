#ifndef _KERNEL_DESCRIPTOR_TABLE_INCLUDE
#define _KERNEL_DESCRIPTOR_TABLE_INCLUDE

#include <stdint.h>

#define IDT_DESC_BIT16 0x06
#define IDT_DESC_BIT32 0x0E
#define IDT_DESC_RING1 0x40
#define IDT_DESC_RING2 0x20
#define IDT_DESC_RING3 0x60
#define IDT_DESC_PRESENT 0x80

struct kernel_gdt_entry_struct {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed));
typedef struct kernel_gdt_entry_struct kernel_gdt_entry;

typedef struct {
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

typedef struct {
	kernel_idt_entry Entries[256];
} kernel_idt;

typedef struct {
	uint32_t ds;	// Data segment selector
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t interrupt_id, error_code;
	uint32_t eip, cs, eflags, useresp, ss;
} registers;

void gdt_initialize(kernel_gdt *GDT);
void idt_initialize(kernel_idt *IDT);

void idt_set_entry(kernel_idt *IDT, uint8_t Index, uint32_t Base, uint16_t Selector, uint8_t Flags); 

#endif //_KERNEL_DESCRIPTOR_TABLE_INCLUDE
