
typedef struct {
  uint16_t offset_0_15;
  uint16_t code_segment_selector;
  uint8_t ist;
  uint8_t type_and_attributes;
  uint16_t offset_16_31;
  uint32_t offset_32_63;
  uint32_t null_uint32;
} IDT_Entry;

//NOTE(Torin: 2016-08-24) An error_code is always pushed
//on to the stack in our execption and interrupt handling model
typedef struct {
  Register_State register_state;
  uint64_t interrupt_number;
  uint64_t error_code;
  uint64_t rip;
  uint64_t cs;  
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} __attribute((packed)) Interrupt_Stack_Frame;

static void irq_handler_keyboard();
static void irq_handler_pit();
static void lapic_timer_interrupt();
static void lapic_periodic_timer_interrupt_handler(void);

void idt_install_all_interrupts();
void set_interrupt_handler(uint64_t interrupt_number, void *handler_address);
void idt_encode_entry(const uintptr_t entry_addr, const uintptr_t handler_addr, const bool is_user_accessible);
void idt_install_interrupt(IDT_Entry *idt, const uint32_t irq_number, const uint64_t irq_handler_addr);