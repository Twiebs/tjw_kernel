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

static void 
isr_handler_page_fault(ISRRegisterState register_state) {
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
	
	klog_error("Page Fault (%s%s%s%s%s, faulting_address: %lu)", 
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
		redraw_log_if_dirty(&globals.log);
		asm volatile ("hlt");
	} else {
		//TODO(TORIN) This was the userspace application lets kill it
    klog_error("UNHANDLED USERSPACE VIOLATION!!!");
		redraw_log_if_dirty(&globals.log);
		asm volatile ("hlt");
	}
}



export void isr_common_handler(ISRRegisterState regstate) {
	klog_debug("software interrupt recieved: %u", regstate.interrupt_number);
	klog_debug("error_code: %u", regstate.error_code);
	klog_debug("rip: %u", regstate.return_rip);

	if (regstate.interrupt_number == 14) {
		isr_handler_page_fault(regstate);
	}
}

extern void 
irq_common_handler(IRQRegisterState regstate) {
	if (_interrupt_handlers[regstate.interrupt_number] != 0) {
	  InterruptHandlerProc proc = (InterruptHandlerProc)_interrupt_handlers[regstate.interrupt_number];
		proc();
	} else if (regstate.interrupt_number == 0xFFFFFFFF) {
		klog_error("INVALID INTERRUPT");
		return;
	} else {
		klog_error("Uninialized interrupt handler!");
	}
  lapic_write_register(globals.lapic_address, 0xB0, 0x00);
}

static void 
irq_handler_keyboard(void) {
  static const uint32_t KEYBOARD_STATUS_PORT = 0x64;
  static const uint32_t KEYBOARD_DATA_PORT   = 0x60;
	uint8_t keyboard_status = read_port(KEYBOARD_STATUS_PORT);
  if(keyboard_status & 0x01) {
		int8_t keycode = read_port(KEYBOARD_DATA_PORT);
		if (keycode < 0) return;

    //TODO(Torin) Need to store keymap and event processing system
    //to deferr processing of keyboard events to when the aplication 
    //is actualy running independent of the interrupt handler
		if(keycode == KEYCODE_BACKSPACE_PRESSED){
      klog_remove_last_input_character(&globals.log);
		} else if (keycode == KEYCODE_ENTER_PRESSED){
      klog_submit_input_to_shell(&globals.log);
		} else if (keycode == KEYCODE_UP_PRESSED) {
			//_kterm.scroll_count = -1;
		} else if (keycode == KEYCODE_DOWN_PRESSED) {
			//_kterm.scroll_count = 1;
		} else {
      klog_add_input_character(&globals.log, keyboard_map[keycode]);
		}
  }
}

static void 
irq_handler_pit(void) {
  //klog_debug("local APIC timer interupt");
	redraw_log_if_dirty(&globals.log);
}