#ifndef _KERNEL_DEBUG_INCLUDE
#define _KERNEL_DEBUG_INCLUDE

#include "kernel_types.h"
#include "kernel_asm.h"

inline void debug_play_speaker(uint32_t frequency) {
	outb(0x61, (uint8_t)(0x03| (uint8_t)(frequency << 2)));
}

#endif//_KERNEL_DEBUG_INCLUDE
