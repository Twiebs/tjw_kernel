
struct HardwareInteruptRegisterState {
	uint32_t ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_number;
	uint32_t eip, cs, eflags, useresp, ss;
};

struct SoftwareInterruptRegisterState {
	uint32_t ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_number, error_code;
	uint32_t eip, cs, eflags, useresp, ss;
};

typedef void(*InterruptHandlerProc)(void);

global_variable InterruptHandlerProc interrupt_handlers[256 - 32];

internal inline 
void isr_handler_page_fault(SoftwareInterruptRegisterState register_state) {
	uint32_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

	int present = !(register_state.error_code & 0x1);
	int is_read_only = (register_state.error_code & 0x2);
	int is_user_mode = (register_state.error_code & 0x4);
	int is_reserved_overwrite = (register_state.error_code & 0x8);
	int id = register_state.error_code & 0x10;

	kerror("Page Fault (%s %s %s %s)", 
			present ? "present" : "",
			is_read_only ? "read-only" : "",
			is_user_mode ? "user-mode" : "",
			is_reserved_overwrite ? "reserved" : "");

	asm volatile("hlt");
}

export void isr_common_handler(SoftwareInterruptRegisterState regstate) {
	kdebug("software interrupt recieved: %u", regstate.int_number);
	kdebug("error_code: %u", regstate.error_code);
	kdebug("eip: %u", regstate.eip);

	if (regstate.error_code == 14) {
		isr_handler_page_fault(regstate);
	}

}

export void irq_common_handler(HardwareInteruptRegisterState regstate) {
	//kdebug("interrupt recieved: %u", regstate.int_number);

	if (interrupt_handlers[regstate.int_number] != 0) {
	  InterruptHandlerProc proc = interrupt_handlers[regstate.int_number];
		proc();
	} else if (regstate.int_number == 0xFFFF) {
		klog("UHANDLED INTERRUPT");
		return;
	} else {
		klog("Uninialized interrupt handler!");
	}

	 static const uint8_t PIC1_COMMAND_PORT = 0x20;
	 static const uint8_t PIC2_COMMAND_PORT = 0xA0;
	 static const uint8_t PIC_EOI_CODE = 0x20;

	 if (regstate.int_number < 8) {
		 write_port(PIC1_COMMAND_PORT, PIC_EOI_CODE);
	 } else {
			write_port(PIC2_COMMAND_PORT, PIC_EOI_CODE);
	 }
}

internal void 
irq_handler_keyboard(void) {
  static const uint32_t KEYBOARD_STATUS_PORT = 0x64;
  static const uint32_t KEYBOARD_DATA_PORT   = 0x60;
	uint8_t keyboard_status = read_port(KEYBOARD_STATUS_PORT);
  if(keyboard_status & 0x01) {
		int8_t keycode = read_port(KEYBOARD_DATA_PORT);
		if (keycode < 0) return;

		if (keycode == KEYCODE_BACKSPACE_PRESSED) {
			if (input_buffer_count > 0) {
				input_buffer[input_buffer_count] = 0;
				input_buffer_count -= 1;
				is_input_buffer_dirty = true;
			}
		} else if (keycode == KEYCODE_ENTER_PRESSED) {
			is_command_ready = true;
		} else {
			input_buffer[input_buffer_count++] = keyboard_map[(uint32_t)keycode];
			input_buffer[input_buffer_count] = 0;
			is_input_buffer_dirty = true;
		}
  }
}

global_variable uint32_t kernel_tick_count = 0;
global_variable uint32_t kernel_supertick_count = 0;

internal void 
irq_handler_pit(void) {
	kterm_redraw_if_required();
}
