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
void isr_handler_page_fault(ISRRegisterState register_state) 
{
	static const uint64_t CAUSE_PROTECTION_VIOLATION_OR_NOT_PRESENT = (1 << 0);
	static const uint64_t CAUSE_WRITE_OR_READ = (1 << 1);
	static const uint64_t CAUSE_USER_OR_KERNEL = (1 << 2);
	static const uint64_t CAUSE_RESERVED_BIT_SET = (1 << 3);
	static const uint64_t CAUSE_INSTRUCTION_FETCH = (1 << 4);

	uintptr_t faulting_address;
	asm volatile ("movq %%cr2, %0" : "=r"(faulting_address));
	uint64_t is_protection_voloation_else_not_present = register_state.error_code & CAUSE_PROTECTION_VIOLATION_OR_NOT_PRESENT;
	uint64_t is_write_else_read = register_state.error_code & CAUSE_WRITE_OR_READ;
	uint64_t is_usermode_else_kernel = register_state.error_code & CAUSE_USER_OR_KERNEL;
	uint64_t is_reserved_bit_overwritten = register_state.error_code & CAUSE_RESERVED_BIT_SET;
	uint64_t is_instruction_else_data = register_state.error_code & CAUSE_INSTRUCTION_FETCH;
	
	kerror("Page Fault (%s%s%s%s%s, faulting_address: %lu)", 
    (is_protection_voloation_else_not_present ? "caused by protection violation, " : "page is non-present, "),
    (is_write_else_read ? "cause by page write, " : "caused by page read, "),
    (is_usermode_else_kernel ? "happened in user-mode, " : "happened in kernel-mode, "),
    (is_reserved_bit_overwritten ? "a reserved bit was overrwriten, " : "reserved bits are fine, "),
    (is_instruction_else_data ? "caused by instruction fetch" : "caused by data access"),
    (faulting_address));
  print_virtual_address_info_2MB(faulting_address);

	if (is_usermode_else_kernel == false) {
		//This is a serious bug there should never be a page-fault in the kernel
		//TODO(Torin) Need better painic mechanisim that handles this sort of thing manualy
		//and mabye drops back into real mode to go back to old-school vga text buffer and
		//does a blue-screen of death type of deal to insure that the error is reported properly

    log_page_info();

		kterm_redraw_if_required(&_kterm, &_iostate);
		asm volatile ("hlt");
	} else {
		//TODO(TORIN) This was the userspace application lets kill it
    klog("UNHANDLED USERSPACE VIOLATION!!!");
		kterm_redraw_if_required(&_kterm, &_iostate);
		asm volatile ("hlt");
	}
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
