
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


void gdt_encode_system_descriptor(uintptr_t base_address, uint32_t segment_limit, uint8_t type, uint8_t privlege_level, uintptr_t out) {
  kassert(segment_limit < (1 << 19));
  kassert(type < (1 << 4));
  kassert(privlege_level < (1 << 2));

  struct GDT_System_Descriptor {
    uint16_t segment_limit_0_15;
    uint16_t base_address_0_15;
    uint8_t base_address_16_23;
    uint8_t type : 4;
    uint8_t zero : 1;
    uint8_t privlege_level : 2;
    uint8_t present : 1;
    uint8_t segment_limit_16_19 : 4;
    uint8_t available : 1;
    uint8_t reserved0 : 2; 
    uint8_t granularity : 1;
    uint16_t base_address_24_31;
    uint32_t base_address_32_63;
    uint32_t reserved1;
  } __attribute__((packed));

  struct GDT_System_Descriptor descriptor = {};
  descriptor.base_address_0_15  = (base_address >> 0) & 0xFFFF;
  descriptor.base_address_16_23 = (base_address >> 16) & 0xFF;
  descriptor.base_address_24_31 = (base_address >> 24) & 0xFF;
  descriptor.base_address_32_63 = (base_address >> 32) & 0xFFFFFFFF;

  descriptor.segment_limit_0_15 = (segment_limit >> 0) & 0xFFFF;
  descriptor.segment_limit_16_19 = (segment_limit >> 16) & 0b1111;

  descriptor.present = 1;
  descriptor.available = 1;
  descriptor.type = type;
  descriptor.privlege_level = privlege_level;
  memcpy(out, &descriptor, sizeof(struct GDT_System_Descriptor));

  //klog_debug("tss base address: 0x%X", base_address);
  //klog_debug("tss limit: 0x%X", segment_limit);
}

void tss_ldr(const uint16_t selector_index) {
  asm volatile("cli");

  struct TSS_Selector {
    union {
      struct {
        uint16_t requestor_prilvilege_level : 2;
        uint16_t table_indicator : 1; //NOTE(Torin) Must always be zero
        uint16_t selector_index : 13;
      };
       uint16_t packed;
    };
  } task_register;
  
  //@Research
  //TODO(Torin: 2016-08-24) AMD64 system manual has the task_register with the above structure
  //It appears that is incorrect and its just a flat selector_index.  Make sure this is the case!
  uint16_t value = selector_index | 0b00; //This is probably what was ment?
  klog_debug("task_register selector_index: 0x%X", (uint32_t)value);
  //asm volatile ("ltr %0" : : "a"(task_register.packed));
  asm volatile ("ltr %0" : : "a"(value));
  asm volatile("sti");
}