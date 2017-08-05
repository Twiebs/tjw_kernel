
extern uint64_t GDT64;

static inline
void kdebug_log_gdt(){
  uint64_t *gdt = (uint64_t *)&GDT64;

  klog_debug("GDT:");
  for(size_t i = 0; i < 6; i++){
    klog_debug(" 0x%X: 0x%X", i*0x8, gdt[i]);
  }  
}

static inline
void kdebug_log_system_cpu_info(System_Info *sys){
  for (size_t i = 0; i < sys->total_cpu_count; i++) {
    klog_info("CPU%lu: lapic_id: %u", i, (uint32_t)sys->cpu_lapic_ids[i]);
  }
}

static void
kdebug_ioapic_log_irq_map(uintptr_t ioapic_base){
  for(size_t i = 0; i < 8; i++){
    IOAPIC_IRQR_LOW irqr_low = {};
    //IOAPIC_IRQR_HIGH irqr_high = {};
    irqr_low.packed = ioapic_read_register(ioapic_base, 0x10 + (i * 2));
    //irqr_high.packed = ioapic_read_register(ioapic_base, 0x10 + (i * 2) + 1);
    klog_debug("IRQ %u", (uint32_t)i);
    klog_debug("  vector: %u", (uint32_t)irqr_low.vector);
    klog_debug("  mask %u", (uint32_t)irqr_low.mask);
  }
}

void kdebug_log_interrupt_stack_frame(Interrupt_Stack_Frame *frame) {
  Register_State *state = &frame->register_state;
  klog_debug("interrupt_register_state:");
  klog_debug(" rax: 0x%X, r8: 0x%X", state->rax, state->r8);
  klog_debug(" rbx: 0x%X, r9: 0x%X", state->rbx, state->r9);
  klog_debug(" rcx: 0x%X, r10: 0x%X", state->rcx, state->r10);
  klog_debug(" rdx: 0x%X, r11: 0x%X", state->rdx, state->r11);
  klog_debug(" rdi: 0x%X, r12: 0x%X", state->rdi, state->r12);
  klog_debug(" rsi: 0x%X, r13: 0x%X", state->rsi, state->r13);
  klog_debug(" rbp: 0x%X, r14: 0x%X", state->rbp, state->r14);
  klog_debug(" rsp: 0x%X, r15: 0x%X", frame->rsp, state->r15);
  klog_debug(" rip: 0x%X", frame->rip);
}