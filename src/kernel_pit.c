#include "kernel_types.h"
#include "kernel_descriptor_table.h"
#include "kernel_pic.h"

#define		I86_PIT_OCW_MASK_BINCOUNT			1	//00000001
#define		I86_PIT_OCW_MASK_MODE				0xE	//00001110
#define		I86_PIT_OCW_MASK_RL				0x30	//00110000
#define		I86_PIT_OCW_MASK_COUNTER			0xC0	//11000000

#define		I86_PIT_OCW_BINCOUNT_BINARY	0	//0		//! Use when setting I86_PIT_OCW_MASK_BINCOUNT
#define		I86_PIT_OCW_BINCOUNT_BCD	1	//1
#define		I86_PIT_OCW_MODE_TERMINALCOUNT	0	//0000		//! Use when setting I86_PIT_OCW_MASK_MODE
#define		I86_PIT_OCW_MODE_ONESHOT	0x2	//0010
#define		I86_PIT_OCW_MODE_RATEGEN	0x4	//0100
#define		I86_PIT_OCW_MODE_SQUAREWAVEGEN	0x6	//0110
#define		I86_PIT_OCW_MODE_SOFTWARETRIG	0x8	//1000
#define		I86_PIT_OCW_MODE_HARDWARETRIG	0xA	//1010
#define		I86_PIT_OCW_RL_LATCH		0	//000000	//! Use when setting I86_PIT_OCW_MASK_RL
#define		I86_PIT_OCW_RL_LSBONLY		0x10	//010000
#define		I86_PIT_OCW_RL_MSBONLY		0x20	//100000
#define		I86_PIT_OCW_RL_DATA		0x30	//110000
#define		I86_PIT_OCW_COUNTER_0		0	//00000000	//! Use when setting I86_PIT_OCW_MASK_COUNTER
#define		I86_PIT_OCW_COUNTER_1		0x40	//01000000
#define		I86_PIT_OCW_COUNTER_2		0x80	//10000000

#define		I86_PIT_REG_COUNTER0		0x40
#define		I86_PIT_REG_COUNTER1		0x41
#define		I86_PIT_REG_COUNTER2		0x42
#define		I86_PIT_REG_COMMAND		0x43

#if 0
static uint32_t g_pit_ticks;

extern pit_irq() {
	asm ("add %esp, 12");
	asm ("pushad");
	g_pit_ticks++;	
	outb(X86_PCI1_REG_COMMAND, I86_PI86_PIC_OCW2_MASK_EOI);
	asm ("popad");
	asm ("iretd");
}


#endif

#if 0 
static void i86_pit_start_counter(uint32_t freq, uint8_t counter, uint8_t mode) {
	// kassert(freq != 0);
	uint16_t divisor = uint16_t(1193181 / (uint16_t)freq);

	uint8_t ocw = 0;
	ocw |= mode;
	ocw |= I86_PIC_OCW_RL_DATA;
	ocw |= counter;
	outb(I86_PIT_REG_COMMAND, ocw);
	outb(I86_PIT_REG_COUNTER0, divisor & 0xFF);
	outb(I86_PIT_REG_COUNTER0, (divisor >> 8) & 0xFF;

	g_pit_ticks = 0;	
}
#endif

		
// void i86_pit_initialize(kernel_idt *IDT) {
	// idt_set_entry(IDT, 32, (uint32_t)pit_irq, );
// }
