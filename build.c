#define BUILD_DEBUG
#define FORCE_REDRAW_ON_LOG_ENTRY 1

#pragma GCC diagnostic error "-Wimplicit-function-declaration"
#pragma GCC diagnostic error "-Wshadow"
#pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wextra"

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"

#ifdef BUILD_DEBUG 
#define COMPILER_FLAGS -g -O0 
#else//BUILD_DEBUG
#define COMPILER_FLAGS -g -O2
#endif//BUILD_DEBUG

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

//Utilities
#include "src/utils/error_code.h"
#include "src/utils/math.h"
#include "src/utils/memory.h"
#include "src/utils/string.h"
#include "src/utils/persistent_block_allocator.h"


//Main System
#include "src/tasking/lock.h"

#include "src/kernel_pci.h"
#include "src/kernel_memory.h"
#include "src/system/descriptor_tables.h"
#include "src/system.h"
#include "src/kernel.h"
#include "src/assembly.h"
#include "src/hardware_keyboard.h"
#include "src/kernel_log.h"


#include "src/elf64.h"	
#include "src/system/interrupt.h"

//Shell
#include "src/shell/command_line_shell.h"
#include "src/shell/shell_commands.h"

//Filesystem
#include "src/filesystem/virtual_filesystem.h"
#include "src/filesystem/storage_device.h"
#include "src/filesystem/ext2_filesystem.h"

#include "src/utils/buffered_file_reader.h"

#include "src/filesystem/ext2_debug.h"

//Graphics
#include "src/graphics/vga_mode_3.h"
#include "src/graphics/display.h"
#include "src/graphics/software_renderer.h"
#include "src/graphics/graphics_device.h"
#include "src/graphics/bochs_graphics_device.h"
#include "src/graphics/intel.h"

//Desktop
#include "src/desktop_enviroment/desktop_enviroment.h"
#include "src/tasking/elf64_loader.h" 

//USB devices and drivers
#include "src/usb/usb.h"
#include "src/usb/ehci.h"
#include "src/usb/usb_device.h"

//Debug Procedures
#include "src/kernel_debug.h"
#include "src/memory_debug.h"
#include "src/graphics/display_debug.h"

//  Source Files
//===========================

//System
#include "src/kernel_entry.c"
#include "src/system/interrupt.c"
#include "src/kernel_log.c"
#include "src/system.c" 
#include "src/kernel_memory.c"

//Utilities Source Files
#include "src/utils/math.c"
#include "src/utils/memory.c"
#include "src/utils/string.c"
#include "src/utils/persistent_block_allocator.c"
#include "src/utils/buffered_file_reader.c"

//Filesystem Source files
#include "src/filesystem/virtual_filesystem.c"
#include "src/filesystem/ext2_filesystem.c"
#include "src/filesystem/storage_device.c"

//Shell Source files
#include "src/shell/command_line_shell.c"
#include "src/shell/shell_commands.c"

//Graphics Source files
#include "src/graphics/vga_mode_3.c"
#include "src/graphics/display.c"
#include "src/graphics/software_renderer.c"
#include "src/graphics/bochs_graphics_device.c"
#include "src/graphics/intel.c"

//Desktop
#include "src/desktop_enviroment/desktop_enviroment.c"
#include "src/tasking/elf64_loader.c"

//USB
#include "src/usb/usb_protocol.c"
#include "src/usb/ehci.c"

//Debug
#include "src/graphics/display_debug.c"
#include "src/kernel_memory_debug.c"