
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
  //the pic are masked out; however, the timer interrupt is left active
  //because it will be used later to calibrate the lapic. TODO This should
  //probably be disabled here and then re-enabled when the lapic is calibrated.
  write_port_uint8(PIC1_DATA_PORT, 0b11111101);
  write_port_uint8(PIC2_DATA_PORT, 0b11111111);
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