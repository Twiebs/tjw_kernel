#if 0
echo =========================================================================================================
gcc -DBUILD_IS_RUNNING -E build.c | sh
exit
#endif

#define BUILD_DEBUG
#define FORCE_REDRAW_ON_LOG_ENTRY 1

#pragma GCC diagnostic error "-Wimplicit-function-declaration"

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
  bin/bin_to_txt trampoline.bin src/trampoline.txt

	gcc COMPILER_FLAGS -m64 -mno-red-zone -nostdlib -ffreestanding -I../tjw_kernel/src -c build.c -o build.c.o
	nasm -felf64 src/primary_cpu_init.asm -isrc/ -o primary_cpu_init.asm.o
	nasm -felf64 src/x86_64_entry.asm -isrc/ -o x86_64_entry.asm.o
  ld -melf_x86_64 -n -T kernel_link.ld -o bin/kernel build.c.o x86_64_entry.asm.o primary_cpu_init.asm.o
  objcopy --only-keep-debug bin/kernel bin/debug_symbols
	//objcopy --strip-debug bin/kernel bin/kernel

	rm *.o
  rm *.bin
	exit
#endif

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

//Utilities
#include "src/utils/error_code.h"
#include "src/utils/math.h"
#include "src/utils/memory.h"
#include "src/utils/string.h"


#include "src/descriptor_tables.h"
#include "src/kernel_synch.h"
#include "src/kernel.h"
#include "src/hardware_keyboard.h"
#include "src/kernel_log.h"




#include "src/kernel_memory.h"

#include "src/interrupt_handler.h"

#include "src/kernel_pci.h"
#include "src/system.h"

//Shell
#include "src/shell/command_line_shell.h"
#include "src/shell/shell_commands.h"

//Filesystem
#include "src/filesystem/virtual_filesystem.h"
#include "src/filesystem/storage_device.h"
#include "src/filesystem/ext2_filesystem.h"
#include "src/filesystem/ext2_debug.h"


//Graphics
#include "src/graphics/vga_mode_3.h"
#include "src/graphics/vesa_framebuffer.h"

//USB devices and drivers
#include "src/usb/usb.h"
#include "src/usb/ehci.h"
#include "src/usb/usb_device.h"
  
#include "src/kernel_debug.h"
#include "src/kernel_task.h"

//  Source Files
//===========================

#include "src/kernel_entry.c"
#include "src/interrupt_handler.c"
#include "src/kernel_log.c"
#include "src/system.c" 

//Utilities Source Files
#include "src/utils/math.c"
#include "src/utils/memory.c"
#include "src/utils/string.c"

//Filesystem Source files
#include "src/filesystem/virtual_filesystem.c"
#include "src/filesystem/ext2_filesystem.c"
#include "src/filesystem/storage_device.c"

//Shell Source files
#include "src/shell/command_line_shell.c"
#include "src/shell/shell_commands.c"

//Graphics Source files
#include "src/graphics/vga_mode_3.c"
#include "src/graphics/vesa_framebuffer.c"