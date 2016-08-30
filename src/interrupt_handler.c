
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
typedef void(*ExceptionHandlerProc)(Interrupt_Stack_Frame);

static ExceptionHandlerProc g_exception_handlers[] = {
  0x0, 
  0x0,
  0x0,
  0x0,
  0x0,
  0x0,
  0x0,
  0x0,
  0x0,
  0x0,
  0x0,
  0x0,
  0x0,
  isr_handler_general_protection_fault,
  isr_handler_page_fault,
  0x0,
  0x0,
};

static void kprocess_destroy(uint64_t pid){
  klog_debug("destroyed process: %lu", pid);
  kgfx_draw_log_if_dirty(&globals.log);
  asm volatile("hlt");
}

static void syscall_handler_print_string(const char *string, size_t length){
  klog_write_string(&globals.log, string, length);
}

static void syscall_handler_exit_process(Interrupt_Stack_Frame_No_Error stack){
  //NOTE(Torin 2016-08-29) Set the stackframe to send us back into the kernel
  stack.ss = GDT_RING0_DATA;
  stack.cs = GDT_RING0_CODE;
  stack.rip = (uintptr_t)kprocess_destroy;
  stack.rsp = (uintptr_t)globals.system_info.kernel_stack_address;
  asm volatile("mov $0x00, %rdi");
}

const uintptr_t g_syscall_procedures[] = {
  (uintptr_t)syscall_handler_print_string,
  (uintptr_t)syscall_handler_exit_process,
};

extern void 
isr_common_handler(Interrupt_Stack_Frame stack) {
	klog_debug("Exception Occured:%u %s", stack.interrupt_number, EXCEPTION_NAMES[stack.interrupt_number]);
	klog_debug("error_code: %u", stack.error_code);
	klog_debug("rip: 0x%X", stack.rip);

  ExceptionHandlerProc exception_handler = g_exception_handlers[stack.interrupt_number];
  if(exception_handler != 0){
    exception_handler(stack);
  }
}

extern void 
irq_common_handler(IRQRegisterState regstate) {
  if(_interrupt_handlers[regstate.interrupt_number] == 0x00){
		klog_error("unregistered interrupt handler");
  } else {
	  InterruptHandlerProc proc = (InterruptHandlerProc)_interrupt_handlers[regstate.interrupt_number];
		proc();
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
  bochs_magic_breakpoint;
  Keyboard_State *keyboard = &globals.keyboard;
  if(globals.keyboard.scancode_event_stack_count > 0){
    klog_process_keyevents(keyboard, &globals.log); 
    if(keyboard->scancode_event_stack == keyboard->scancode_event_stack0){
      keyboard->scancode_event_stack = keyboard->scancode_event_stack1;
    } else { keyboard->scancode_event_stack = keyboard->scancode_event_stack0; }
    memset(keyboard->scancode_event_stack, 0x00, sizeof(keyboard->scancode_event_stack0));
    keyboard->scancode_event_stack_count = 0;
  }

	kgfx_draw_log_if_dirty(&globals.log);
}