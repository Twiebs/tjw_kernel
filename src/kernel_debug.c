
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
  for(size_t i = 0; i < sys->cpu_count; i++){
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

static void 
kdebug_log_interrupt_stack_frame(Interrupt_Stack_Frame *frame){
  Register_State *state = &frame->register_state;
  klog_debug("rax: 0x%X, r8: 0x%X", state->rax, state->r8);
  klog_debug("rbx: 0x%X, r9: 0x%X", state->rbx, state->r9);
  klog_debug("rcx: 0x%X, r10: 0x%X", state->rcx, state->r10);
  klog_debug("rdx: 0x%X, r11: 0x%X", state->rdx, state->r11);
  klog_debug("rdi: 0x%X, r12: 0x%X", state->rdi, state->r12);
  klog_debug("rsi: 0x%X, r13: 0x%X", state->rsi, state->r13);
  klog_debug("rbp: 0x%X, r14: 0x%X", state->rbp, state->r14);
  klog_debug("rsp: 0x%X, r15: 0x%X", frame->rsp, state->r15);
  klog_debug("rip: 0x%X", frame->rip);
}

static void
kdebug_log_page_table_entry_info(const uintptr_t entry){
  uintptr_t physical_address = entry & ~0xFFF;
  bool is_present = entry & PAGE_PRESENT_BIT;
  bool is_writeable = entry & PAGE_WRITEABLE_BIT;
  bool access_mode = entry & PAGE_USER_ACCESS_BIT;
  bool is_write_through_caching = entry & PAGE_WRITE_TROUGH_CACHE_BIT;
  bool is_caching_disabled = entry & PAGE_DISABLE_CACHE_BIT;
  bool is_huge_page = entry & PAGE_HUGE_BIT;
  klog_debug("physical_address: 0x%X", physical_address);
  klog_debug("present: %s", is_present ? "true" : "false");
  klog_debug("writeable: %s", is_writeable ? "true" : "false");
  klog_debug("user accessiable: %s", access_mode ? "true" : "false");
  klog_debug("write through caching: %s", is_write_through_caching ? "true" : "false");
  klog_debug("caching disabled: %s", is_caching_disabled ? "true" : "false");
  klog_debug("huge page: %s", is_huge_page ? "true" : "false"); 
}

static void 
kdebug_log_virtual_address_info_2MB(const uintptr_t virtual_address){
  uint64_t p4_index = (virtual_address >> 39) & 0x1FF;
  uint64_t p3_index = (virtual_address >> 30) & 0x1FF;
  uint64_t p2_index = (virtual_address >> 21) & 0x1FF;
  uint64_t offset   = (virtual_address >> 0)  & 0xFFFFF;

  klog_debug("logging virtual address info");
  klog_debug(" virtual_address: 0x%X", virtual_address);
  klog_debug(" p4_index: %lu", p4_index);
  klog_debug(" p3_index: %lu", p3_index);
  klog_debug(" p2_index: %lu", p2_index);
  klog_debug(" offset: %lu", offset);

  klog_debug(" p4_entry_info:");
  kdebug_log_page_table_entry_info(g_p4_table.entries[p4_index]);
  klog_debug(" p3_entry_info:");
  kdebug_log_page_table_entry_info(g_p3_table.entries[p3_index]);
  klog_debug(" p2_entry_info:");
  kdebug_log_page_table_entry_info(g_p2_table.entries[p2_index]);
}

static void
kdebug_log_page_info() {
  for(size_t i = 0; i < g_current_page_index; i++){
    bool is_present = g_p2_table.entries[i] & 0b01;
    uintptr_t physical_address = g_p2_table.entries[i] & ~(0b111111111111);
    uintptr_t virtual_address = i * 1024 * 1024 * 2;
    klog_debug("page_entry: virtual %lu mapped to %lu", virtual_address, physical_address);
  }
}
