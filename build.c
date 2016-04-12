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
	nasm -felf64 x86_64_boot.asm -o x86_64_boot.asm.o
	nasm -felf64 x86_64_entry.asm -o x86_64_entry.asm.o
	gcc DEBUG_FLAGS -m64 -mno-red-zone -nostdlib -ffreestanding -c build.c -o build.c.o
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

#include "kernel.h"
#include "kernel_io.h"

#include "utility.h"
#include "filesystem_kernel.h"
#include "hardware_keyboard.h"
#include "video_vga_text.h"
#include "interrupt_handlers.h"

#include "x86_64_entry.c"
#include "interrupt_handler.c"
#include "kernel_io.c"
#include "video_vga_text.c"
#include "filesystem_kernel.cpp"
#endif
