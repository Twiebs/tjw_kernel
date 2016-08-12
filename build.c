#if 0
echo =========================================================================================================
gcc -DBUILD_IS_RUNNING -E build.c | sh
exit
#endif

//#define BUILD_DEBUG

#pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wextra"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#ifdef BUILD_DEBUG 
#define COMPILER_FLAGS -g -O0
#else//BUILD_DEBUG
#define COMPILER_FLAGS -g -O2
#endif//BUILD_DEBUG

#if defined(BUILD_IS_RUNNING)
  nasm -fbin src/secondary_cpu_init.asm -o trampoline.bin
  tools/bin_to_txt trampoline.bin src/trampoline.txt

	gcc COMPILER_FLAGS -m64 -mno-red-zone -nostdlib -fforward-propagate -fmerge-constants -ffreestanding -I../tjw_kernel/src -c build.c -o build.c.o
	nasm -felf64 src/primary_cpu_init.asm -isrc/ -o primary_cpu_init.asm.o
	nasm -felf64 src/x86_64_entry.asm -isrc/ -o x86_64_entry.asm.o
  ld -melf_x86_64 -n -T kernel_link.ld -o bin/kernel build.c.o x86_64_entry.asm.o primary_cpu_init.asm.o
  objcopy --only-keep-debug bin/kernel bin/debug_symbols
	objcopy --strip-debug bin/kernel bin/kernel
	rm *.o
	exit
#endif

#ifndef DBUILD_IS_RUNNING
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

#include "src/utility.h"

#include "src/kernel.h"
#include "src/kernel_synch.h"
#include "src/kernel_shell.h"
#include "src/kernel_log.h"
#include "src/kernel_graphics.h"
#include "src/kernel_memory.h"

#include "src/hardware_keyboard.h"
#include "src/interrupt_handler.h"

#include "src/x86_64_entry.c"
#include "src/interrupt_handler.c"

#include "src/kernel_log.c"
#include "src/kernel_shell.c"


#endif
