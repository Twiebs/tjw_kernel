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

extern void 
isr_common_handler(ISRRegisterState regstate) {
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
  lapic_write_register(globals.system_info.lapic_virtual_address, 0xB0, 0x00);
}


#define KEYBOARD_SCANCODE1_LSHIFT 0x2A
#define KEYBOARD_SCANCODE1_RSHIFT 0x36

static const char SCANCODE_TO_LOWERCASE_ACII[] = {
  0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 
  0,  'q', 'w', 'e', 'r',	't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,	
  0,	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,	'\\', 
  'z', 'x', 'c', 'v', 'b', 'n',	'm', ',', '.', '/',  0, 0, 0,	' ',	
};

static const char SCANCODE_TO_UPERCASE_ACII[] = {
  0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 
  0,  'Q', 'W', 'E', 'R',	'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,	
  0,	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,	'|', 
  'Z', 'X', 'C', 'V', 'B', 'N',	'M', '<', '>', '?',  0, 0, 0,	' ',	
};

//TODO(Torin) This will not handle modifiers correctly
//if the modifier key is pressed after the original key
//was already down it will still register that the key was
//pressed with the modifier.  Does this event matter?  Would this be better?
//Should store keycode + extra btye saying (isdown, shiftdown?, ctrldow?, altdown?)

static void 
irq_handler_keyboard(void) {
  static const uint32_t KEYBOARD_DATA_PORT   = 0x60;
  static const uint32_t KEYBOARD_STATUS_PORT = 0x64;
	uint8_t keyboard_status = read_port_uint8(KEYBOARD_STATUS_PORT);
  if(keyboard_status & 0x01){
		uint8_t scancode = read_port_uint8(KEYBOARD_DATA_PORT);
    if(globals.log_keyboard_events){
      klog_debug("[keyboard_event] scancode %u", scancode);
    }

    //TODO(Torin) How should this case be handled?  Is it even possbile to occur in practice?
    //What if the user has a stupidly slow computer?
    if(globals.keyboard.scancode_event_stack_count >= sizeof(globals.keyboard.scancode_event_stack)) {
      klog_warning("[keyboard_event] keyboard event stack reached maximum size before events were processed");
      return;
    }

    if (scancode > 0x80) {  //NOTE(Torin)key release event 
      uint8_t base_scancode = scancode - 0x80;
      globals.keyboard.keystate[base_scancode] = 0;
    } else { //NOTE(Torin)Key press event
      globals.keyboard.keystate[scancode] = 1;
    }
    
    //TODO(Torin) we would be saving some subtractions cost in userspace by storing the keycode here instead of the
    //scanconde and adding an extra byte to store information about the isDown state and keyboard modifiers!
    globals.keyboard.scancode_event_stack[globals.keyboard.scancode_event_stack_count++] = scancode;
  }
}

static void 
klog_process_keyevents(Keyboard_State *keyboard, Circular_Log *log){
  for(size_t i = 0; i < keyboard->scancode_event_stack_count; i++){
    uint8_t scancode = keyboard->scancode_event_stack[i];

    if(scancode == KEYCODE_BACKSPACE_PRESSED){
      klog_remove_last_input_character(log);
		} else if(scancode == KEYCODE_ENTER_PRESSED){
      klog_submit_input_to_shell(log);
		} else if (scancode == KEYCODE_UP_PRESSED){
      if(log->scroll_offset < log->current_entry_count - 1){
        log->scroll_offset += 1;
      }


      log->is_dirty = true;
    } else if (scancode == KEYCODE_DOWN_PRESSED){
      if(log->scroll_offset > 0){
        log->scroll_offset -= 1;
        log->is_dirty = true;
      }
    }

    if(scancode < (int)sizeof(SCANCODE_TO_LOWERCASE_ACII)){
      char ascii_character = 0;
      if(keyboard->keystate[KEYBOARD_SCANCODE1_LSHIFT] ||
        keyboard->keystate[KEYBOARD_SCANCODE1_RSHIFT]){
        ascii_character = SCANCODE_TO_UPERCASE_ACII[scancode];
      } else { ascii_character = SCANCODE_TO_LOWERCASE_ACII[scancode]; }
      if(ascii_character == 0) return;
      klog_add_input_character(log, ascii_character);
    }
  }
}

static void 
irq_handler_pit(void) {
  Keyboard_State *keyboard = &globals.keyboard;
  if(globals.keyboard.scancode_event_stack_count > 0){
    klog_process_keyevents(keyboard, &globals.log); 
    if(keyboard->scancode_event_stack == keyboard->scancode_event_stack0){
      keyboard->scancode_event_stack = keyboard->scancode_event_stack1;
    } else { keyboard->scancode_event_stack = keyboard->scancode_event_stack0; }
    memset(keyboard->scancode_event_stack, 0x00, sizeof(keyboard->scancode_event_stack0));
    keyboard->scancode_event_stack_count = 0;
  }
	redraw_log_if_dirty(&globals.log);
}
