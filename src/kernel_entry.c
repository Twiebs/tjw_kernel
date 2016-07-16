#if 0

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define export extern "C"
#define foreign extern "C"
#define external extern "C"
#define internal static 
#define global_variable static 

#define kassert(expr, msg) stdout_write_fmt(&_iostate, msg)
#define kerror(...) stdout_write_fmt(&_iostate, __VA_ARGS__)
#define klog(...) stdout_write_fmt(&_iostate, __VA_ARGS__)
#define kdebug(...) stdout_write_fmt(&_iostate, __VA_ARGS__)
#define kpanic(...) stdout_write_fmt(&_iostate, __VA_ARGS__); kterm_redraw_if_required(&_kterm, &_iostate)

#define DEBUG_LOG(msg) bochs_write_string(msg)

#include "filesystem_kernel.h"
#include "hardware_keyboard.h"
#include "utility.h"

#define COM1_PORT 0x3F8
#define COM2_PORT 0x2F8
#define COM3_PORT 0x3E8
#define COM4_PORT 0x2E8

external uint8_t read_port(uint16_t port);
external void write_port(uint16_t port, uint8_t data);

//=================================================
typedef struct {
	KFS_Header *kfs;
	KFS_Node *kfs_nodes;
	uint8_t *base_data;
} FileSystem;

typedef struct {
	const char *top_entry;
	uint32_t top_entry_index;
	int32_t scroll_count;
	uint32_t last_entry_count;
} VGATextTerm; 

typedef struct {
	uint8_t *framebuffer;
	uint16_t resolutionX;
	uint16_t resolutionY;
	uint16_t depth;
} VideoState;

global_variable IOState _iostate;
global_variable FileSystem _fs;
global_variable VGATextTem _kterm;

//===================================================

internal void 
stdout_write_fmt(IOState *io, const char *fmt, ...);

internal void kterm_redraw_if_required(VGATextTerm *kterm, IOState *io);

external uint8_t read_port(uint16_t port);
external void write_port(uint16_t port, uint8_t data);

//===================================================


internal inline
void bochs_write_string(const char *str) {
	size_t length = strlen(str);
	for (size_t i = 0; i < length; i++) {
		write_port(COM1_PORT, str[i]);
	}
}

#ifdef BUILD_SINGLE_TRANSLATION_UNIT
#include "descriptor_table.c"
#include "kernel_feature.c"
#include "filesystem_kernel.cpp"
#include "interrupt_handler.c"

#if defined(BUILD_KERNEL_X86_32)
#include "x86_32_entry.cpp"
#elif defined(BUILD_KERNEL_X86_64)
#include "x86_64_entry.c"
#endif //X86_32 or X86_64
#endif //SINGLE_TRANSLATION_UNIT

internal void
kernel_reboot() {
	struct {
			uint16_t size;
			uintptr_t address;
	} __attribute__((packed)) idtr = { 0, 0 };
	asm volatile ("lidt %0" : : "m"(idtr));
	asm volatile ("int $0x3");
}

#define COMMAND_UNKNOWN_COMMAND 1
#define COMMAND_INVALID_ARG_COUNT 2

internal int 
is_command_signature_valid(const char *input, size_t input_len,
		const char *command_name, size_t command_length,
		uint32_t expected_arg_count, uint32_t actual_arg_count) 
{
	if (strings_match(input, input_len, command_name, command_length)) {
		if (expected_arg_count != actual_arg_count) {
			klog("%s expected %u arguments, only %u were provided", command_name, expected_arg_count, actual_arg_count);
			return COMMAND_INVALID_ARG_COUNT;
		} else {
			return 0;
		}
	}
	return COMMAND_UNKNOWN_COMMAND;
}


internal bool 
process_shell_command(const char *command) {
		const char *procedure_name = command;
		size_t arg_count = 0;
		const char *args[6];
		size_t arg_lengths[6];

		const char *current = command;
		while (is_char_alpha(*current)) current++;
		size_t procedure_name_length = current - procedure_name;
		while (*current == ' ') {
			current++; 
			args[arg_count] = current;
			while (*current != ' ' && *current != 0) current++;
			arg_lengths[arg_count] = current - args[arg_count];
			arg_count++;
		}
		
#define command_hook(name, argc)  \
		else if ((command_validity_status = is_command_signature_valid(procedure_name, procedure_name_length,  \
						name, LITERAL_STRLEN(name), argc, arg_count)))

		bool command_was_handled = false;
		int command_validity_status = 0;

		if (0) {}
		command_hook("reboot", 0) { kernel_reboot(); }

		command_hook("ls", 0) {
			for (uint32_t i = 0; i < _fs.kfs->node_count; i++) {
				KFS_Node *node = &_fs.kfs_nodes[i];
				klog("filename: %s, filesize: %u", node->name, node->size);
			}
		}

		command_hook("print", 1) {
			KFS_Node *node = kfs_find_file_with_name(args[0], arg_lengths[0]);
			if (node == 0) {
				klog("file %.*s could not be found", arg_lengths[0], args[0]);
			} else {
				const uint8_t *node_data = _fs.base_data + node->offset;
				klog("[%s]: %.*s", node->name, node->size, node_data);
			}
		}



		if (command_validity_status == COMMAND_UNKNOWN_COMMAND) {
			klog("unknown command '%s'", command);
			return false;
		} else if (command_validity_status == COMMAND_INVALID_ARG_COUNT) {
			return false;
		}


		return true;
}


internal void
kterm_redraw_if_required(VGATextTerm *kterm, IOState *io) {
	static const uint8_t VGA_TEXT_COLUMN_COUNT = 80;
	static const uint8_t VGA_TEXT_ROW_COUNT = 25;
	static uint8_t *VGA_TEXT_BUFFER = (uint8_t*)(0xB8000);
	static const uint32_t VGA_TEXT_INDEX_MAX = VGA_TEXT_COLUMN_COUNT * VGA_TEXT_ROW_COUNT;

	if (kterm->top_entry == 0) {
		kterm->top_entry = io->output_buffer;
		kterm->top_entry_index = 0;
		kterm->scroll_count = 0;
		kterm->last_entry_count = 0;
	}
	
	if (kterm->scroll_count != 0) {
		int32_t ammount_to_scroll = kterm->scroll_count;
		int32_t index_after_scroll = (int32_t)kterm->top_entry_index + kterm->scroll_count;
		if (index_after_scroll < 0) {
			ammount_to_scroll = -kterm->top_entry_index;
		} 

			if (ammount_to_scroll > 0) {
				while (*kterm->top_entry != 0) kterm->top_entry++;
				kterm->top_entry++;
				kterm->top_entry_index++;
				io->is_output_buffer_dirty = true;
			} else if (ammount_to_scroll < 0){
				kterm->top_entry -= 2;
				while (*kterm->top_entry != 0) kterm->top_entry--;
				kterm->top_entry++;
				kterm->top_entry_index--;
				io->is_output_buffer_dirty = true;
			}

		kterm->scroll_count = 0;
	}
	
	if (io->is_command_ready) {
		process_shell_command(io->input_buffer);
		io->is_command_ready = false;
		io->input_buffer_count = 0;
		memset(io->input_buffer, 0, sizeof(IOState::input_buffer));
		io->is_input_buffer_dirty = true;
	}

  if (io->is_output_buffer_dirty) {
		memset(VGA_TEXT_BUFFER, 0, (VGA_TEXT_COLUMN_COUNT * VGA_TEXT_ROW_COUNT) * 2);
		uint8_t current_color = VGAColor_GREEN;
		uint8_t current_row = 0, current_column = 0;
		
		const char *read = kterm->top_entry;
		while (*read != 0 && current_row < 25) {
			size_t index = ((current_row * VGA_TEXT_COLUMN_COUNT) + current_column) * 2;
			VGA_TEXT_BUFFER[index] = *read;
			VGA_TEXT_BUFFER[index+1] = current_color;
			current_column++;
			if (current_column > VGA_TEXT_COLUMN_COUNT) {
				current_column = 2;
				current_row++;
			}
			read++;
			if (*read == 0) {
				read++;
				current_row++;
				current_column = 0;
			} 
		}
		io->is_output_buffer_dirty = false;
	}

	if (io->is_input_buffer_dirty) {
		uint32_t current_row = VGA_TEXT_ROW_COUNT - 1;
		uint32_t current_column = 0;
		uint32_t index = ((current_row * VGA_TEXT_COLUMN_COUNT) + current_column) * 2;

		VGA_TEXT_BUFFER[index] = '>';
		VGA_TEXT_BUFFER[index+1] = VGAColor_GREEN;
		index += 2;
		current_column += 1;
	
		for (uint32_t i = 0; i < io->input_buffer_count; i++) {
		  VGA_TEXT_BUFFER[index] = io->input_buffer[i];
			VGA_TEXT_BUFFER[index+1] = VGAColor_GREEN;
			index += 2;
			current_column++;
		}

		uint32_t diff = VGA_TEXT_COLUMN_COUNT - current_column;
		for (uint32_t i = 0; i < diff; i+=2) {
			VGA_TEXT_BUFFER[index] = 0;
			VGA_TEXT_BUFFER[index+1] = 0;
			index += 2;
		}

		io->is_input_buffer_dirty = false;
	}
}

//====================================================================
//===========================================================================

#if 0
internal inline
void safemode_checks() {

#define support_check(x,func) \
	if (func()) { klog(x " : SUPPORTED"); } \
	else { klog(x " : UNSUPPORTED"); } \

	klog("[System Info]");

	support_check("cpuid", cpuid_is_supported);
	support_check("apic", cpuid_is_apic_supported);
	support_check("longmode", cpuid_is_longmode_supported);
	klog("=============================================");
}
#endif

#endif