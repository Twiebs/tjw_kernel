#if 0
gcc -DBUILD_IS_RUNNING -E build.c | sh
exit
#endif

#define BUILD_DEBUG
#define BUILD_SINGLE_TRANSLATION_UNIT 
#define BUILD_KERNEL_X86_64

#pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wextra"
#ifdef BUILD_DEBUG 
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#ifdef BUILD_DEBUG
#define DEBUG_FLAGS -g -O0
#else
#define DEBUG_FLAGS
#endif

#if defined(BUILD_IS_RUNNING)
	nasm -felf64 src/x86_64_boot.asm 	-isrc/ -o x86_64_boot.asm.o
	nasm -felf64 src/x86_64_entry.asm -isrc/ -o x86_64_entry.asm.o
	gcc DEBUG_FLAGS -m64 -mno-red-zone -nostdlib -ffreestanding -I../tjw_kernel/src -c build.c -o build.c.o
	ld -melf_x86_64 -n -T kernel_link.ld -o bin/kernel x86_64_boot.asm.o x86_64_entry.asm.o build.c.o
#if defined(BUILD_DEBUG)
	objcopy --only-keep-debug bin/kernel bin/debug_symbols
	objcopy --strip-debug bin/kernel bin/kernel
#endif
	rm *.o
	exit
#endif

#ifndef DBUILD_IS_RUNNING
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

#include "src/kernel.h"
#include "src/kernel_io.h"

#include "src/utility.h"
#include "src/filesystem_kernel.h"
#include "src/hardware_keyboard.h"
#include "src/video_vga_text.h"
#include "src/interrupt_handlers.h"

#include "src/x86_64_entry.c"
#include "src/interrupt_handler.c"
#include "src/kernel_io.c"
#include "src/video_vga_text.c"
#include "src/filesystem_kernel.cpp"
#endif
