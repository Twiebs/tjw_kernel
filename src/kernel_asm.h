#ifndef _KERNEL_ASM_INCLUDE
#define _KERNEL_ASM_INCLUDE

#include "kernel_types.h"

//Writes out a byte to an I/O location
inline void outb(uint16_t port, uint8_t value) {
	asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Reads in a byte from the I/O Port
inline uint8_t inb(uint16_t port) {
	uint8_t result;
	asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
	return result;
}



// TODO(Torin) Consider switching to this method of 
// setting the intterupt table it might be better then requiring
// to create the descriptor table structs for the gdt and idt 

// Calls lidt with the provided base pointer and 
// size of the intterupt table
inline void lidt(void *base, uint16_t size){
	struct {
		uint16_t length;
		uint16_t base;
	} __attribute__((packed)) IDTR;

	IDTR.length = size;
	IDTR.base = (uint32_t) base;
	asm ("lidt (%0)" : : "p"(&IDTR));
}

#endif // KERNEL_ASM_INCLUDE
