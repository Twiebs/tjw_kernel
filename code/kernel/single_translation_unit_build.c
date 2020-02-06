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
#include "utils/error_code.h"
#include "utils/math.h"
#include "utils/memory.h"
#include "utils/string.h"
#include "utils/persistent_block_allocator.h"


//Main System
#include "tasking/lock.h"

#include "kernel_pci.h"
#include "kernel_memory.h"
#include "system/descriptor_tables.h"
#include "system.h"
#include "kernel.h"
#include "assembly.h"
#include "hardware_keyboard.h"
#include "kernel_log.h"


#include "elf64.h"	
#include "system/interrupt.h"

//Shell
#include "shell/command_line_shell.h"
#include "shell/shell_commands.h"

//Filesystem
#include "filesystem/virtual_filesystem.h"
#include "filesystem/storage_device.h"
#include "filesystem/ext2_filesystem.h"

#include "utils/buffered_file_reader.h"

#include "filesystem/ext2_debug.h"

//Graphics
#include "graphics/vga_mode_3.h"
#include "graphics/display.h"
#include "graphics/software_renderer.h"
#include "graphics/graphics_device.h"
#include "graphics/bochs_graphics_device.h"
#include "graphics/intel.h"

//Desktop
#include "desktop_enviroment/desktop_enviroment.h"
#include "tasking/elf64_loader.h" 

//USB devices and drivers
#include "usb/usb.h"
#include "usb/usb_ehci.h"
#include "usb/usb_device.h"

//Debug Procedures
#include "kernel_debug.h"
#include "memory_debug.h"
#include "graphics/display_debug.h"

//  Source Files
//===========================

//System
#include "kernel_entry.c"
#include "system/interrupt.c"
#include "kernel_log.c"
#include "system.c" 
#include "kernel_memory.c"

//Utilities Source Files
#include "utils/math.c"
#include "utils/memory.c"
#include "utils/string.c"
#include "utils/persistent_block_allocator.c"
#include "utils/buffered_file_reader.c"

//Filesystem Source files
#include "filesystem/virtual_filesystem.c"
#include "filesystem/ext2_filesystem.c"
#include "filesystem/storage_device.c"

//Shell Source files
#include "shell/command_line_shell.c"
#include "shell/shell_commands.c"

//Graphics Source files
#include "graphics/vga_mode_3.c"
#include "graphics/display.c"
#include "graphics/software_renderer.c"
#include "graphics/bochs_graphics_device.c"
#include "graphics/intel.c"

//Desktop
#include "desktop_enviroment/desktop_enviroment.c"
#include "tasking/elf64_loader.c"

//USB
#include "usb/usb_protocol.c"
#include "usb/debug_usb.c"
#include "usb/debug_usb_ehci.c"
#include "usb/usb_ehci.c"

//Debug
#include "graphics/display_debug.c"
#include "kernel_memory_debug.c"