
struct InteruptRegisterState {
	uint32_t ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_number;
	uint32_t eip, cs, eflags, useresp, ss;
};

typedef void(*InterruptHandlerProc)(void);

global_variable InterruptHandlerProc interrupt_handlers[256 - 32];

export void irq_common_handler(InteruptRegisterState regstate) {
	//kdebug("interrupt %u", regstate.int_number);
	//kterm_redraw_if_required();

	if (interrupt_handlers[regstate.int_number] != 0) {
	  InterruptHandlerProc proc = interrupt_handlers[regstate.int_number];
		proc();
	} else if (regstate.int_number == 0xFFFF) {
		klog("UHANDLED INTERRUPT");
		return;
	} else {
		klog("Uninialized interrupt handler!");
	}
		
	 if (regstate.int_number < 8) {
		 write_port(PIC1_COMMAND_PORT, PIC_EOI_CODE);
	 } else {
			write_port(PIC2_COMMAND_PORT, PIC_EOI_CODE);
	 }
}


//TODO(Torin)
//The kernel terminal keyprinting stuff
//should be deffered to the bottom half
//of the interrupt handler

internal void 
irq_handler_keyboard(void) {
  static const uint32_t KEYBOARD_STATUS_PORT = 0x64;
  static const uint32_t KEYBOARD_DATA_PORT   = 0x60;
  write_port(PIC1_COMMAND_PORT, PIC_EOI_CODE);
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

		//TODO(Torin) This needs to be called later
		kterm_redraw_if_required();
  }
}

global_variable uint32_t kernel_tick_count = 0;
global_variable uint32_t kernel_supertick_count = 0;

internal void 
irq_handler_pit(void) {
	klog("kernel tick");
	kterm_redraw_if_required();
}


