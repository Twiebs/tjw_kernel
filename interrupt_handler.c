typedef struct {
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
	uint64_t interrupt_number;
	uint64_t error_code;
	uint32_t padding_0;
	uint64_t return_rip;
	uint16_t return_cs;
	uint16_t padding_1;
	uint32_t padding_2;
	uint64_t return_rflags;
	uint64_t return_rsp;
	uint16_t return_ss;
	uint16_t padding_3;
	uint32_t padding_4;
} __attribute__((packed)) ISRRegisterState; 

typedef struct {
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
	uint64_t interrupt_number;
	uint32_t padding_0;
	uint64_t return_rip;
	uint16_t return_cs;
	uint16_t padding_1;
	uint32_t padding_2;
	uint64_t return_rflags;
	uint64_t return_rsp;
	uint16_t return_ss;
	uint16_t padding_3;
	uint32_t padding_4;
} __attribute__((packed)) IRQRegisterState; 

typedef void(*InterruptHandlerProc)(void);

internal inline 
void isr_handler_page_fault(ISRRegisterState register_state) {
	uint32_t faulting_address;
	//asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

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
	kterm_redraw_if_required(&_kterm, &_iostate);
	asm volatile ("hlt");
}

export void isr_common_handler(ISRRegisterState regstate) {
	kdebug("software interrupt recieved: %u", regstate.interrupt_number);
	kdebug("error_code: %u", regstate.error_code);
	kdebug("rip: %u", regstate.return_rip);

	if (regstate.interrupt_number == 14) {
		isr_handler_page_fault(regstate);
	}
}

export void irq_common_handler(IRQRegisterState regstate) {
	//kdebug("interrupt recieved: %u", regstate.interrupt_number);

	if (_interrupt_handlers[regstate.interrupt_number] != 0) {
	  InterruptHandlerProc proc = (InterruptHandlerProc)_interrupt_handlers[regstate.interrupt_number];
		proc();
	} else if (regstate.interrupt_number == 0xFFFFFFFF) {
		klog("INVALID INTERRUPT");
		return;
	} else {
		klog("Uninialized interrupt handler!");
	}

	 static const uint8_t PIC1_COMMAND_PORT = 0x20;
	 static const uint8_t PIC2_COMMAND_PORT = 0xA0;
	 static const uint8_t PIC_EOI_CODE = 0x20;

	 //TODO(Torin) This might be wrong
	 if (regstate.interrupt_number < 8) {
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
			if (_iostate.input_buffer_count > 0) {
				_iostate.input_buffer[_iostate.input_buffer_count] = 0;
				_iostate.input_buffer_count -= 1;
				_iostate.is_input_buffer_dirty = true;
			}
		} else if (keycode == KEYCODE_ENTER_PRESSED) {
			_iostate.is_command_ready = true;
		} else if (keycode == KEYCODE_UP_PRESSED) {
			_kterm.scroll_count = -1;
		} else if (keycode == KEYCODE_DOWN_PRESSED) {
			_kterm.scroll_count = 1;
		} else {
			_iostate.input_buffer[_iostate.input_buffer_count++] = keyboard_map[(uint32_t)keycode];
			_iostate.input_buffer[_iostate.input_buffer_count] = 0;
			_iostate.is_input_buffer_dirty = true;
		}
#if 0
		klog("keycode: %u", (uint32_t)keycode);
#endif
  }
}

internal void 
irq_handler_pit(void) {
	kterm_redraw_if_required(&_kterm, &_iostate);
}
