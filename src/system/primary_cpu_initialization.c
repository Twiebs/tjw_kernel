
//NOTE(Torin, 2017-10-01) The kernel currently relies on information provided by a 
//multiboot2 compliant bootloaded(in this case GRUB is used). This is most likely
//temporary and a custom bootloader will be written at a later time. To make that transiton
//more seamless information about the system is abstracted out into a seprate mechanism and
//kernel initialization uses that instead of relying on beening booted by a multiboot2 bootloader
typedef struct {
  uintptr_t rsdp_physical_address;
  int rsdp_version;
} Primary_CPU_Initialization_Info;


//NOTE(Torin, 2017-10-01) This procedure is required to disable the legacy
//PIC chip/emulation. The kernel uses both the LAPIC and IOAPIC instead.
static inline void remap_and_disable_legacy_pic() {
  //NOTE(Torin) Remap the legacy PIC8259 and mask out the interrupt vectors 
  static const uint8_t PIC1_COMMAND_PORT = 0x20;
  static const uint8_t PIC2_COMMAND_PORT = 0xA0;
  static const uint8_t PIC1_DATA_PORT = 0x21;
  static const uint8_t PIC2_DATA_PORT = 0xA1;
  //Initalization Command Words (ICW)
  static const uint8_t ICW1_INIT_CASCADED = 0x11;
  static const uint8_t ICW2_PIC1_IRQ_NUMBER_BEGIN = 0x20;
  static const uint8_t ICW2_PIC2_IRQ_NUMBER_BEGIN = 0x28;
  static const uint8_t ICW3_PIC1_IRQ_LINE_2 = 0x4;
  static const uint8_t ICW3_PIC2_IRQ_LINE_2 = 0x2;
  static const uint8_t ICW4_8068 = 0x01;
  //ICW1 Tells PIC to wait for 3 more words
  write_port_uint8(PIC1_COMMAND_PORT, ICW1_INIT_CASCADED);
  write_port_uint8(PIC2_COMMAND_PORT, ICW1_INIT_CASCADED);
  //ICW2 Set PIC Offset Values
  write_port_uint8(PIC1_DATA_PORT, ICW2_PIC1_IRQ_NUMBER_BEGIN);
  write_port_uint8(PIC2_DATA_PORT, ICW2_PIC2_IRQ_NUMBER_BEGIN);
  //ICW3 PIC Cascading Info
  write_port_uint8(PIC1_DATA_PORT, ICW3_PIC1_IRQ_LINE_2);
  write_port_uint8(PIC2_DATA_PORT, ICW3_PIC2_IRQ_LINE_2);
  //ICW4 Additional Enviroment Info
  //NOTE(Torin) Currently set to 80x86
  write_port_uint8(PIC1_DATA_PORT, ICW4_8068);
  write_port_uint8(PIC2_DATA_PORT, ICW4_8068);
  //NOTE(Torin, 2017-10-01) At this time the interrupts handled by
  //the pic are masked out; however, the timer interrupt will
  //be reenabled when the lapic timer is calibrated
  write_port_uint8(PIC1_DATA_PORT, 0b11111111);
  write_port_uint8(PIC2_DATA_PORT, 0b11111111);
}


static inline void configure_legacy_pit_timer() {
  static const uint8_t PIT_CHANNEL0_DATA_PORT = 0x40;
  static const uint8_t PIT_COMMAND_PORT = 0x43;
  static const uint8_t COMMAND_CHANNEL0 = (0b00) << 6;
  static const uint8_t COMMAND_ACCESS_LOW_AND_HIGH = (0b11) << 4;
  static const uint8_t COMMAND_MODE_SQUARE_WAVE = 0x06; 
  static const uint8_t COMMAND_MODE_RATE_GENERATOR = 0x04; 
  static const uint32_t PIT_BASE_FREQUENCY = 1193182;
  static const uint8_t COUNTER0 = 0x00;
  static const uint8_t RW_LOW_HIGH_MODE = 0x30;

  uint32_t requested_frequency = 1000;
  kassert(requested_frequency > 18);
  kassert(requested_frequency < 1193181);
  //set timer mode
  write_port_uint8(PIT_COMMAND_PORT, COUNTER0 | COMMAND_MODE_RATE_GENERATOR | RW_LOW_HIGH_MODE);
  //Set the divisor
  uint32_t divisor = PIT_BASE_FREQUENCY / requested_frequency;
  uint8_t divisor_low = (uint8_t)(divisor & 0xFF);
  uint8_t divisor_high = (uint8_t)((divisor >> 8) & 0xFF);
  write_port_uint8(PIT_CHANNEL0_DATA_PORT, divisor_low);
  write_port_uint8(PIT_CHANNEL0_DATA_PORT, divisor_high);
}


static inline void lapic_initialize(Virtual_Address lapic_virtual_address) {    
  static const uint64_t LAPIC_SIVR_REGISTER = 0xF0;
  static const uint32_t LAPIC_SIVR_ENABLE = 1 << 8;
  static const uint32_t LAPIC_VERSION_REGISTER = 0x30;
  asm volatile("cli");
  lapic_write_register(lapic_virtual_address, LAPIC_SIVR_REGISTER, 0x31 | LAPIC_SIVR_ENABLE);
  asm volatile("sti");
  uint32_t lapic_version_register = lapic_read_register(lapic_virtual_address, LAPIC_VERSION_REGISTER);
  uint32_t lapic_version = lapic_version_register & 0xFF;
  uint32_t lapic_max_lvt_entries = (lapic_version_register >> 16) & 0xFF;
}

static const uint32_t LAPIC_TIMER_IRQ_NUMBER_REGISTER = 0x320;
static const uint32_t LAPIC_TIMER_INITAL_COUNT_REGISTER = 0x380;
static const uint32_t LAPIC_TIMER_CURRENT_COUNT_REGISTER = 0x390;
static const uint32_t LAPIC_TIMER_DIVIDE_CONFIG_REGISTER = 0x3E0;
static const uint32_t LAPIC_TIMER_IRQ_MASKED = 1 << 16;
static const uint32_t LAPIC_TIMER_DIVIDE_2 = 0b00;
static const uint32_t LAPIC_TIMER_DIVIDE_4 = 0b01;
static const uint32_t LAPIC_TIMER_DIVIDE_8 = 0b10;
static const uint32_t LAPIC_TIMER_DIVIDE_16 = 0b11;
static const uint32_t LAPIC_TIMER_PERODIC_MODE = 0x20000;

void lapic_configure_timer(Virtual_Address lapic_virtual_address) {
  lapic_write_register(lapic_virtual_address, LAPIC_TIMER_DIVIDE_CONFIG_REGISTER, LAPIC_TIMER_DIVIDE_2);
  lapic_write_register(lapic_virtual_address, LAPIC_TIMER_INITAL_COUNT_REGISTER, 0xFFFFFFFF);
  lapic_write_register(lapic_virtual_address, LAPIC_TIMER_IRQ_NUMBER_REGISTER, 0x22 | LAPIC_TIMER_IRQ_MASKED);
}

static inline void calibrate_lapic_timer_frequency(Virtual_Address lapic_virtual_address) {
  //NOTE(Torin) Unmask the PIT timer interrupt
  static const uint8_t PIC1_DATA_PORT = 0x21; 
  write_port_uint8(PIC1_DATA_PORT, 0b11111110);
  //NOTE(Torin) Count how many ticks the LAPIC decrements in 32 milliseconds
  globals.pit_timer_ticks = 0;
  while (globals.pit_timer_ticks <= 32) { asm volatile("nop"); }
  uint32_t ticks_remaining = lapic_read_register(lapic_virtual_address, LAPIC_TIMER_CURRENT_COUNT_REGISTER);  
  uint32_t ticks_in_32_ms = 0xFFFFFFFF - ticks_remaining;
  uint32_t ticks_per_millisecond = ticks_in_32_ms / 32;
  if (ticks_per_millisecond == 0) {
    klog_error("error initializing lapic timer");
    return;
  }
  //NOTE(Torin) Mask the PIT timer interrupt
  write_port_uint8(PIC1_DATA_PORT, 0b11111111);
}


static inline void multiboot2_get_elf_section_info(struct multiboot_tag_elf_sections *elf_sections) {
  development_only_runtime_assert(elf_sections != NULL);
  development_only_runtime_assert(elf_sections->type == MULTIBOOT_TAG_TYPE_ELF_SECTIONS);
  ELF_Section_Header *string_table_section = ((ELF_Section_Header *)elf_sections->sections) + elf_sections->shndx;
  const char *string_table = (const char *)string_table_section->virtualAddress;
  for (size_t i = 0; i < elf_sections->num; i++) {
    ELF_Section_Header *section = (ELF_Section_Header *)(elf_sections->sections + (i * elf_sections->entsize));
    const char *name = section->name_offset + string_table;
    klog_debug("%s", name);
  }
}

void initialize_primary_cpu() {

}