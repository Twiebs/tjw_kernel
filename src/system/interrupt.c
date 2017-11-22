
typedef struct {
  Register_State register_state;
  uint64_t interrupt_number;
  uint64_t rip;
  uint64_t cs;  
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} __attribute((packed)) Interrupt_Stack_Frame_No_Error;

typedef struct {
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} Interrupt_Stack_Frame_Basic;

extern void asm_double_fault_handler();
extern void asm_debug_handler();

extern void asm_isr0(void);
extern void asm_isr1(void);
extern void asm_isr2(void);
extern void asm_isr3(void);
extern void asm_isr4(void);
extern void asm_isr5(void);
extern void asm_isr6(void);
extern void asm_isr7(void);
extern void asm_isr9(void);
extern void asm_isr10(void);
extern void asm_isr11(void);
extern void asm_isr12(void);
extern void asm_isr13(void);
extern void asm_isr14(void);
extern void asm_isr15(void);
extern void asm_isr16(void);
extern void asm_isr17(void);
extern void asm_isr18(void);
extern void asm_isr19(void);
extern void asm_isr20(void);
extern void asm_isr21(void);
extern void asm_isr22(void);
extern void asm_isr23(void);
extern void asm_isr24(void);
extern void asm_isr25(void);
extern void asm_isr26(void);
extern void asm_isr27(void);
extern void asm_isr28(void);
extern void asm_isr29(void);
extern void asm_isr30(void);
extern void asm_isr31(void);

static IDT_Entry g_idt[256];
static uintptr_t g_interrupt_handlers[256];

void idt_install_interrupt(IDT_Entry *idt, const uint32_t irq_number, const uint64_t irq_handler_addr) {
  static const uint64_t PRIVILEGE_LEVEL_0 = 0b00000000;
  static const uint64_t PRIVILEGE_LEVEL_3 = 0b01100000;
  static const uint64_t PRESENT_BIT = (1 << 7); 
  
  static const uint64_t TYPE_TASK_GATE_64 = 0x5;
  static const uint64_t TYPE_INTERRUPT_GATE_64 = 0xE;
  static const uint64_t TYPE_TRAP_GATE_64 = 0xF;

  idt[irq_number].offset_0_15 = (uint16_t)(irq_handler_addr & 0xFFFF);
  idt[irq_number].offset_16_31 = (uint16_t)((irq_handler_addr >> 16) & 0xFFFF);
  idt[irq_number].offset_32_63 = (uint32_t)((irq_handler_addr >> 32) & 0xFFFFFFFF);
  idt[irq_number].type_and_attributes = PRESENT_BIT | TYPE_INTERRUPT_GATE_64 | PRIVILEGE_LEVEL_0; 
  idt[irq_number].code_segment_selector = GDT_RING0_CODE_ENTRY_OFFSET;
  idt[irq_number].ist = 0;
}

void set_interrupt_handler(uint64_t interrupt_number, void *handler_address) {
  g_interrupt_handlers[interrupt_number] = (uintptr_t)handler_address;
}

void idt_encode_entry(const uintptr_t entry_addr, const uintptr_t handler_addr, const bool is_user_accessible) {
  static const uint64_t PRIVILEGE_LEVEL_0 = 0b00000000;
  static const uint64_t PRIVILEGE_LEVEL_3 = 0b01100000;
  static const uint64_t PRESENT_BIT = (1 << 7); 
  
  static const uint64_t TYPE_TASK_GATE_64 = 0x5;
  static const uint64_t TYPE_INTERRUPT_GATE_64 = 0xE;
  static const uint64_t TYPE_TRAP_GATE_64 = 0xF;

  IDT_Entry *entry = (IDT_Entry *)entry_addr;
  entry->offset_0_15 = (uint16_t)(handler_addr & 0xFFFF);
  entry->offset_16_31 = (uint16_t)((handler_addr >> 16) & 0xFFFF);
  entry->offset_32_63 = (uint32_t)((handler_addr >> 32) & 0xFFFFFFFF);
  entry->type_and_attributes = PRESENT_BIT | TYPE_INTERRUPT_GATE_64 | (is_user_accessible ?  PRIVILEGE_LEVEL_3 : PRIVILEGE_LEVEL_0); 
  entry->code_segment_selector = GDT_RING0_CODE_ENTRY_OFFSET; //TODO(Torin: 2017-07-26) This looks wrong0
  entry->ist = 0;
}

void idt_install_all_interrupts() {
  for (uint32_t i = 0; i < 256; i++) {
    idt_install_interrupt(g_idt, i, (uintptr_t)asm_debug_handler);
    g_interrupt_handlers[i] = 0x00;
  }

  idt_install_interrupt(g_idt, 0, (uintptr_t)asm_isr0);
  idt_install_interrupt(g_idt, 1, (uintptr_t)asm_isr1);
  idt_install_interrupt(g_idt, 2, (uintptr_t)asm_isr2);
  idt_install_interrupt(g_idt, 3, (uintptr_t)asm_isr3);
  idt_install_interrupt(g_idt, 4, (uintptr_t)asm_isr4);
  idt_install_interrupt(g_idt, 5, (uintptr_t)asm_isr5);
  idt_install_interrupt(g_idt, 6, (uintptr_t)asm_isr6);
  idt_install_interrupt(g_idt, 7, (uintptr_t)asm_isr7);
  idt_install_interrupt(g_idt, 8, (uintptr_t)asm_double_fault_handler);
  idt_install_interrupt(g_idt, 9, (uintptr_t)asm_isr9);
  idt_install_interrupt(g_idt, 10, (uintptr_t)asm_isr10);
  idt_install_interrupt(g_idt, 11, (uintptr_t)asm_isr11);
  idt_install_interrupt(g_idt, 12, (uintptr_t)asm_isr12);
  idt_install_interrupt(g_idt, 13, (uintptr_t)asm_isr13);
  idt_install_interrupt(g_idt, 14, (uintptr_t)asm_isr14);
  idt_install_interrupt(g_idt, 15, (uintptr_t)asm_isr15);
  idt_install_interrupt(g_idt, 16, (uintptr_t)asm_isr16);
  idt_install_interrupt(g_idt, 17, (uintptr_t)asm_isr17);
  idt_install_interrupt(g_idt, 18, (uintptr_t)asm_isr18);
  idt_install_interrupt(g_idt, 19, (uintptr_t)asm_isr19);
  idt_install_interrupt(g_idt, 20, (uintptr_t)asm_isr20);
  idt_install_interrupt(g_idt, 21, (uintptr_t)asm_isr21);
  idt_install_interrupt(g_idt, 22, (uintptr_t)asm_isr22);
  idt_install_interrupt(g_idt, 23, (uintptr_t)asm_isr23);
  idt_install_interrupt(g_idt, 24, (uintptr_t)asm_isr24);
  idt_install_interrupt(g_idt, 25, (uintptr_t)asm_isr25);
  idt_install_interrupt(g_idt, 26, (uintptr_t)asm_isr26);
  idt_install_interrupt(g_idt, 27, (uintptr_t)asm_isr27);
  idt_install_interrupt(g_idt, 28, (uintptr_t)asm_isr28);
  idt_install_interrupt(g_idt, 29, (uintptr_t)asm_isr29);
  idt_install_interrupt(g_idt, 30, (uintptr_t)asm_isr30);
  idt_install_interrupt(g_idt, 31, (uintptr_t)asm_isr31);

  { //Hardware Interrupts
    static const uint32_t IRQ_PIT = 0x20; 
    static const uint32_t IRQ_KEYBOARD = 0x21;
    extern void asm_irq0(void);
    extern void asm_irq1(void);
    extern void asm_irq2(void);
    extern void asm_irq128(void);

    extern void asm_syscall_handler(void);
    extern void asm_spurious_interrupt_handler(void);

    g_interrupt_handlers[0] = (uintptr_t)irq_handler_pit;
    g_interrupt_handlers[1] = (uintptr_t)irq_handler_keyboard;
    g_interrupt_handlers[2] = (uintptr_t)lapic_timer_interrupt;
    idt_install_interrupt(g_idt, IRQ_PIT, (uintptr_t)asm_irq0);
    idt_install_interrupt(g_idt, IRQ_KEYBOARD, (uintptr_t)asm_irq1);
    idt_encode_entry((uintptr_t)&g_idt[0x22], (uintptr_t)asm_irq2, true);
    idt_encode_entry((uintptr_t)&g_idt[0x80], (uintptr_t)asm_syscall_handler, true);
    idt_encode_entry((uintptr_t)&g_idt[0x31], (uintptr_t)asm_spurious_interrupt_handler, true);
  }

    struct {
    uint16_t limit;
    uintptr_t address;
  } __attribute__((packed)) idtr = { sizeof(g_idt) - 1, (uintptr_t)g_idt };
  asm volatile ("lidt %0" : : "m"(idtr));
  asm volatile ("sti");
}

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
  //Thread_Context *running_thread = globals.task_info.running_threads[cpu_id];
  //uint64_t pid = running_thread->pid;
  //ktask_destroy_process(pid, &globals.task_info); 
  //klog_debug("destroyed pid: %lu", pid);
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
  if(g_interrupt_handlers[stack.interrupt_number] == 0x00){
    klog_error("unregistered interrupt handler");
  } else {
    InterruptHandlerProc proc = (InterruptHandlerProc)g_interrupt_handlers[stack.interrupt_number];
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