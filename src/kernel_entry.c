// NOTE(Torin)
// X86_64 Kernel Entry Point
// This x86_64_kernel_entry is called after
// the computer has been booted with a multiboot2 compliant
// bootloader such as GRUB2 and the bootstrap assembly has put the
// CPU into longmode with a longmode GDT; however, an longmode
// IDT still must be initalized

typedef struct {
  Circular_Log log;
  Command_Line_Shell shell;
  //VGA_Text_Terminal vga_text_term;
  Keyboard_State keyboard;
  Kernel_Memory_State memory_state;
  System_Info system_info;

  Virtual_File_System virtual_file_system;

  Desktop_Enviroment desktop_enviroment;
  Graphics_Device *graphics_device;


  //TODO(Torin 2016-10-24) Dynamic USB_Devices
  USB_Device usb_devices[8];
  uint32_t usb_device_count;
  //TODO(Torin: 2017-07-28) Dynamic Storage_Devices
  Storage_Device storage_devices[8];
  uint64_t storage_device_count;


  volatile uint64_t pit_timer_ticks;
  volatile uint64_t lapic_timer_ticks;

  bool is_logging_disabled;
  bool log_keyboard_events;
} Kernel_Globals;

static Kernel_Globals globals;

void kernel_panic() {
  shell_draw_if_required(&globals.shell, &globals.log);
  while (1) { shell_update(&globals.shell); }
  //asm volatile ("cli");
  //asm volatile ("hlt");
}

//TODO(Torin: 2017-07-28) This is temporary
Storage_Device *create_storage_device() {
  if (globals.storage_device_count > ARRAY_COUNT(globals.storage_devices)) {
    return NULL;
  }

  Storage_Device *result = &globals.storage_devices[globals.storage_device_count++];
  return result;
}

#include "kernel_apic.c"
#include "kernel_acpi.c"
#include "system/descriptor_tables.c"
#include "kernel_exceptions.c"
#include "kernel_pci.c"
#include "kernel_debug.c"
#include "hardware_keyboard.c"
#include "development_diagnostics.h"

static const uint8_t TRAMPOLINE_BINARY[] = {
#include "trampoline.txt"
};



#include "multiboot2.h"
#include "hardware_serial.c"
#include "system/primary_cpu_initialization.c"

extern void ap_entry_procedure(void){
  asm volatile("hlt");
}


void initialize_task_state_segment(CPU_Info *cpu_info) {
  //NOTE(Torin: 2017-07-26) Kernel stack must be initalized
  kassert(cpu_info->kernel_stack_top != 0);
  //TODO(Torin: 2017-07-26) Better names / Comments for TSS Members
  memory_set(&cpu_info->tss, 0x00, sizeof(Task_State_Segment));
  cpu_info->tss.rsp0 = cpu_info->kernel_stack_top;
  //TODO(Torin: 2017-07-27) I think this is unused
  cpu_info->tss.ist1 = cpu_info->kernel_stack_top; 

  //TODO(Torin: 2017-07-27) Consider inlining tss_ldr and refactoring GDT location
  uint8_t *gdt = (uint8_t *)&GDT64;
  gdt_encode_system_descriptor((uintptr_t)&cpu_info->tss, 0xFF, 
    GDT_DESCRIPTOR_TYPE_TSS, 3, (uintptr_t)(gdt + GDT_TSS_ENTRY_OFFSET));
  tss_ldr(GDT_TSS_ENTRY_OFFSET);
}


void initalize_cpu_info_and_start_secondary_cpus(System_Info *system) {
  klog_info("initalizing cpu infos...");
  extern uintptr_t stack_top; //This is the temp stack created in asm
  CPU_Info *cpu_info = &system->cpu_infos[0];
  cpu_info->kernel_stack_top = (uint64_t)&stack_top;
  cpu_info->temporary_memory = memory_allocate_persistent_virtual_pages(2);
  initialize_task_state_segment(cpu_info);


#if 0 //NOTE(Torin) Setup tramponline code and startup SMP processors
  klog_debug("copying trampoline code to 0x1000");
  memcpy(0x1000, TRAMPOLINE_BINARY, sizeof(TRAMPOLINE_BINARY));
  uintptr_t *p4_table_address = (uintptr_t *)0x2000;
  uintptr_t *trampoline_exit = (uintptr_t *)0x2008;
  uintptr_t *cpu_id = (uintptr_t *)0x2010;
  *p4_table_address = (uintptr_t)&g_p4_table;
  *trampoline_exit = (uintptr_t)ap_entry_procedure;
  for(size_t i = 0; i < sys->cpu_count; i++){
    *cpu_id = i;
    //lapic_startup_ap(lapic_virtual_address, sys->cpu_lapic_ids[i], 0x01);
    //TODO(Torin) For now this assumes that the CPU will initalize correctly which is bad
    while(1) {
      spinlock_aquire(&sys->smp_lock);
      if(sys->cpu_count > i) {
        spinlock_release(&sys->smp_lock);
      }
    }
  }
  #endif
}

static inline void extract_initialization_info_from_multiboot2_tags(uint64_t multiboot2_magic, uint64_t multiboot2_address, Primary_CPU_Initialization_Info *initialization_info) {
  if (multiboot2_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    klog_error("the kernel was not booted with a multiboot2 compliant bootloader!");
    kernel_panic();
  }

  if (multiboot2_address & 0x7) {
    klog_error("unaligned multiboot_info!");
    kernel_panic();
  }

    extern uint32_t _KERNEL_END;
    // NOTE(Torin, 2020-01-17) This is where GRUB / multiboot2 puts the kernel.
    static const size_t KERNEL_MULTIBOOT2_ORIGIN = 0x100000;
    initialization_info->kernel_executable_physical_address_start = KERNEL_MULTIBOOT2_ORIGIN;
    initialization_info->kernel_executable_physical_address_end = _KERNEL_END;

  struct multiboot_tag_mmap *mmap_tag = 0;
  struct multiboot_tag *tag = (struct multiboot_tag *)(multiboot2_address + 8);
  while (tag->type != MULTIBOOT_TAG_TYPE_END) {
    switch (tag->type) {

      case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
        struct multiboot_tag_old_acpi *acpi_info = (struct multiboot_tag_old_acpi *)(tag);
        initialization_info->rsdp_physical_address = (uintptr_t)acpi_info->rsdp;
        initialization_info->rsdp_version = 1;
      } break;

      case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
        struct multiboot_tag_new_acpi *acpi_info = (struct multiboot_tag_new_acpi *)(tag);
        initialization_info->rsdp_physical_address = (uintptr_t)acpi_info->rsdp;
        initialization_info->rsdp_version = 2;
      } break;

#if 0
      case MULTIBOOT_TAG_TYPE_ELF_SECTIONS: {
        struct multiboot_tag_elf_sections *elf_sections = (struct multiboot_tag_elf_sections *)(tag);
        multiboot2_get_elf_section_info(elf_sections);
      } break;
#endif    

      case MULTIBOOT_TAG_TYPE_MMAP: mmap_tag = (struct multiboot_tag_mmap *)(tag); break;

    }
    tag = (struct multiboot_tag *)(((uint8_t *)tag) + ((tag->size + 7) & ~7));
  }

  if (mmap_tag != 0) {
    multiboot_memory_map_t *mmap_entry = (multiboot_memory_map_t *)(mmap_tag->entries);
    while((uintptr_t)mmap_entry < (uintptr_t)mmap_tag + mmap_tag->size){
      static const uint8_t MEMORY_AVAILABLE = 1;
      if(mmap_entry->type == MEMORY_AVAILABLE && mmap_entry->addr >= 0x100000){
        Memory_Range *memory_range = &initialization_info->usable_ranges[initialization_info->usable_range_count++];
        memory_range->physical_address = mmap_entry->addr;
        memory_range->size_in_bytes = mmap_entry->len;
        memory_range->physical_4KB_page_count = mmap_entry->len / 4096;
      }
       mmap_entry = (multiboot_memory_map_t *)((uintptr_t)mmap_entry + mmap_tag->entry_size);
    }
  }
}

static inline void kernel_debug_shell_loop() {
  while(1) { 
    shell_update(&globals.shell);
  };
}


extern void kernel_longmode_entry(uint64_t multiboot2_magic, uint64_t multiboot2_physical_address) 
{
	development_only_initialize_serial_port_logging();

  log_info(INITIALIZATION, "Kernel Version %u.%u.%u", KERNEL_VERSION_MAJOR, KERNEL_VERSION_MINOR, KERNEL_VERSION_REVISION);

  //NOTE(Torin 2016-09-02) At this point the kernel has been called into by our
  //bootstrap assembly and interrupts are disabled.  A Longmode GDT has been loaded
  //but the IDT still need to be configured

  remap_and_disable_legacy_pic();
  idt_install_all_interrupts();

  //NOTE(Torin, 2017-10-05) This is a wierd way to get info out of the multiboot bootloader. Not
  //too thriled about how this works for now...
  //This also currently has side-effects! It initializes memory ranges in the memory manager!
  //This is too confusing @refactor
  Primary_CPU_Initialization_Info initialization_info = {};
  extract_initialization_info_from_multiboot2_tags(multiboot2_magic, 
    multiboot2_physical_address, &initialization_info);
  validate_primary_cpu_initialization_info(&initialization_info);

  memory_manager_initialize(&initialization_info);

  shell_initialize(&globals.shell);

  //TODO(Torin, 2017-10-05) It is very confusing to be casting a physical address to a pointer
  //and relying on it being in the memory mapped section of ram. We need to check to see if it's
  //mapped already and have the parse_rsdp procedure just take a integer physical_address
  if (initialization_info.rsdp_version == 1) {
    acpi_parse_root_system_descriptor_version1((RSDP_Descriptor_1*)initialization_info.rsdp_physical_address);
  } else if (initialization_info.rsdp_version == 2) {
    acpi_parse_root_system_descriptor_version2((RSDP_Descriptor_2*)initialization_info.rsdp_physical_address);
  }

  System_Info *system = &globals.system_info;
  system->run_mode = System_Run_Mode_DEBUG_SHELL;

  kassert(system->lapic_physical_address != 0);
  kassert(system->ioapic_physical_address != 0);
  system->lapic_virtual_address = memory_map_physical_mmio(system->lapic_physical_address, 1);
  system->ioapic_virtual_address = memory_map_physical_mmio(system->ioapic_physical_address, 1);

  lapic_initialize(system->lapic_virtual_address);

  set_interrupt_handler(2, lapic_periodic_timer_interrupt_handler);
  lapic_write_register(system->lapic_virtual_address, LAPIC_TIMER_INITAL_COUNT_REGISTER, 0xFFFF);
  lapic_write_register(system->lapic_virtual_address, LAPIC_TIMER_IRQ_NUMBER_REGISTER, 0x22 | LAPIC_TIMER_PERODIC_MODE); 
  ioapic_initalize(system->ioapic_virtual_address);
  set_interrupt_handler(2, lapic_timer_interrupt); 

  initalize_cpu_info_and_start_secondary_cpus(system);

  pci_initialize_default_device_drivers();
  pci_enumerate_and_create_devices();
  pci_initialize_valid_devices();

  if (system->run_mode == System_Run_Mode_DESKTOP_ENVIROMENT) {
    desktop_enviroment_initialize(&globals.desktop_enviroment, globals.graphics_device);
  }

  system->run_mode = System_Run_Mode_DEBUG_SHELL;
  if (system->run_mode == System_Run_Mode_DEBUG_SHELL) {
    kernel_debug_shell_loop();
  } else if (system->run_mode == System_Run_Mode_DESKTOP_ENVIROMENT) {
    while (1) { desktop_enviroment_update(&globals.desktop_enviroment); }
  }
}
