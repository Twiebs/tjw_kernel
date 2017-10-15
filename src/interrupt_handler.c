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

extern void kprocess_destroy(){
  uint32_t cpu_id = cpu_get_id();
  Thread_Context *running_thread = globals.task_info.running_threads[cpu_id];
  uint64_t pid = running_thread->pid;
  ktask_destroy_process(pid, &globals.task_info); 
  klog_debug("destroyed pid: %lu", pid);
  asm volatile("int $0x20");
  while(1) { asm volatile("hlt"); }
}

static uint64_t syscall_handler_print_string(const char *string, size_t length){
  klog_write_fmt(&globals.log, Log_Category_DEFAULT, Log_Level_INFO, "%.*s", length, string);
  return 0;
}

uint64_t syscall_handler_exit_process(volatile Interrupt_Stack_Frame_Basic stack){
  extern void asm_exit_usermode(void);
  uint32_t cpu_id = cpu_get_id();
  stack.ss = GDT_RING0_DATA_ENTRY_OFFSET;
  stack.cs = GDT_RING0_CODE_ENTRY_OFFSET;
  stack.rip = (uintptr_t)asm_exit_usermode;
  stack.rsp = (uintptr_t)globals.system_info.cpu_infos[cpu_id].kernel_stack_top;
  asm volatile("mov $0x00, %rdi");
  return 0;
}

const uintptr_t g_syscall_procedures[] = {
  (uintptr_t)syscall_handler_print_string,
  (uintptr_t)syscall_handler_exit_process,
};

extern void isr_common_handler(Interrupt_Stack_Frame stack) {
  if(g_exception_handlers[stack.interrupt_number] != 0){
    g_exception_handlers[stack.interrupt_number](stack);
  } else {
    klog_debug("Exception Occured:%u %s", stack.interrupt_number, EXCEPTION_NAMES[stack.interrupt_number]);
    klog_debug("error_code: %u", stack.error_code);
    klog_debug("rip: 0x%X", stack.rip);
  }
}

extern void irq_common_handler(Interrupt_Stack_Frame_No_Error stack) {
  if(_interrupt_handlers[stack.interrupt_number] == 0x00){
		klog_error("unregistered interrupt handler");
  } else {
	  InterruptHandlerProc proc = (InterruptHandlerProc)_interrupt_handlers[stack.interrupt_number];
		proc();
  }
  lapic_write_register(globals.system_info.lapic_virtual_address, 0xB0, 0x00);
  static const uint8_t PIC1_COMMAND_PORT = 0x20;
  write_port_uint8(0x20, 0x20);
}


void irq_handler_keyboard() {
  keyboard_state_add_scancodes_from_ps2_device(&globals.keyboard);
}


static void irq_handler_pit(void){
  globals.pit_timer_ticks += 1;
  write_port_uint8(0x20, 0x20);
}

static void lapic_periodic_timer_interrupt_handler(void){
  globals.lapic_timer_ticks += 1;
}


static void lapic_timer_interrupt(void) {
  globals.lapic_timer_ticks += 1;
}