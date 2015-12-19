#include "kernel_pic.h"
#include "kernel_asm.h"

#define X86_PIC1_REG_COMMAND 0x20
#define X86_PIC1_REG_STATUS 0x20
#define X86_PIC1_REG_DATA 0x21
#define X86_PIC1_REG_IMR 0x21

#define X86_PIC2_REG_COMMAND 0xA0
#define X86_PIC2_REG_STATUS 0xA0
#define X86_PIC2_REG_DATA 0xA1
#define X86_PIC2_REG_IMR 0xA1

#define X86_PIC_ICW1_MASK_IC4 0x1 // Set if PIT remap expects ICW4 
#define X86_PIC_ICW1_MASK_SNGL 0x2 // Single or Cascaded
#define X86_PIC_ICW1_MASK_ADI 0x4 // Call Address Interval
#define X86_PIC_ICW1_MASK_LTIM 0x8 //Operation Mode
#define X86_PIC_ICW1_MASK_INIT 0x10 //Initaliztaion Cmd - Should allways be set

#define I86_PIC_ICW4_MASK_UPM		0x1	// Enable 80x86 Mode
#define I86_PIC_ICW4_MASK_AEOI		0x2	// Automatic EOI
#define I86_PIC_ICW4_MASK_MS		0x4	// Slelects Buffer Type
#define I86_PIC_ICW4_MASK_BUF		0x8 // Buffer Mode Enabled	
#define I86_PIC_ICW4_MASK_SFNM		0x10 // Special Fully Nested Mode	

#define kassert(BOOLEAN) 

// TODO(Torin) These routines were much simpler
// When they were just implemented in ASM
// Consider moving more of the implementation directly into
// ASM

// NOTE(Torin) If you forget what some of this is doing check the
// pic.asm file that has this function implemented in asm
// you wrote a lot of notation in it

void x86_pic_initialize(uint8_t base0, uint8_t base1) {
	uint8_t icw = 0;

	// TODO(Torin) This is bizzar why would you try and clear the bits
	// if you already know what these values will be!
	// Just clear the thing to zero before you write into it 
	icw = (icw & ~X86_PIC_ICW1_MASK_INIT) | X86_PIC_ICW1_MASK_INIT;
	icw = (icw & ~X86_PIC_ICW1_MASK_IC4) | X86_PIC_ICW1_MASK_IC4;
	outb(X86_PIC1_REG_COMMAND, icw);
	outb(X86_PIC2_REG_COMMAND, icw);

	outb(X86_PIC1_REG_COMMAND, base0);
	outb(X86_PIC2_REG_COMMAND, base1);

	outb(X86_PIC1_REG_COMMAND, 0x04);
	outb(X86_PIC2_REG_COMMAND, 0x02);

	icw = (icw & ~I86_PIC_ICW4_MASK_UPM) | I86_PIC_ICW4_MASK_UPM;
	outb(X86_PIC1_REG_COMMAND, icw);
	outb(X86_PIC2_REG_COMMAND, icw);
}
