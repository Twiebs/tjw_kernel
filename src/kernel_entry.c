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
#include "descriptor_tables.c"
#include "kernel_exceptions.c"
#include "kernel_pci.c"
#include "kernel_debug.c"
#include "hardware_keyboard.c"
#include "development_diagnostics.h"



static IDT_Entry _idt[256];
static uintptr_t _interrupt_handlers[256];

static const uint8_t TRAMPOLINE_BINARY[] = {
#include "trampoline.txt"
};

static void idt_install_all_interrupts() {
  extern void asm_double_fault_handler();
  extern void asm_debug_handler();
  
	for (uint32_t i = 0; i < 256; i++) {
		idt_install_interrupt(_idt, i, (uintptr_t)asm_debug_handler);
		_interrupt_handlers[i] = 0x00;
	}

	{ //Software Exceptions
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

		idt_install_interrupt(_idt, 0, (uintptr_t)asm_isr0);
		idt_install_interrupt(_idt, 1, (uintptr_t)asm_isr1);
		idt_install_interrupt(_idt, 2, (uintptr_t)asm_isr2);
		idt_install_interrupt(_idt, 3, (uintptr_t)asm_isr3);
		idt_install_interrupt(_idt, 4, (uintptr_t)asm_isr4);
		idt_install_interrupt(_idt, 5, (uintptr_t)asm_isr5);
		idt_install_interrupt(_idt, 6, (uintptr_t)asm_isr6);
		idt_install_interrupt(_idt, 7, (uintptr_t)asm_isr7);
		idt_install_interrupt(_idt, 8, (uintptr_t)asm_double_fault_handler);
		idt_install_interrupt(_idt, 9, (uintptr_t)asm_isr9);
		idt_install_interrupt(_idt, 10, (uintptr_t)asm_isr10);
		idt_install_interrupt(_idt, 11, (uintptr_t)asm_isr11);
		idt_install_interrupt(_idt, 12, (uintptr_t)asm_isr12);
		idt_install_interrupt(_idt, 13, (uintptr_t)asm_isr13);
		idt_install_interrupt(_idt, 14, (uintptr_t)asm_isr14);
		idt_install_interrupt(_idt, 15, (uintptr_t)asm_isr15);
		idt_install_interrupt(_idt, 16, (uintptr_t)asm_isr16);
		idt_install_interrupt(_idt, 17, (uintptr_t)asm_isr17);
		idt_install_interrupt(_idt, 18, (uintptr_t)asm_isr18);
		idt_install_interrupt(_idt, 19, (uintptr_t)asm_isr19);
		idt_install_interrupt(_idt, 20, (uintptr_t)asm_isr20);
		idt_install_interrupt(_idt, 21, (uintptr_t)asm_isr21);
		idt_install_interrupt(_idt, 22, (uintptr_t)asm_isr22);
		idt_install_interrupt(_idt, 23, (uintptr_t)asm_isr23);
		idt_install_interrupt(_idt, 24, (uintptr_t)asm_isr24);
		idt_install_interrupt(_idt, 25, (uintptr_t)asm_isr25);
		idt_install_interrupt(_idt, 26, (uintptr_t)asm_isr26);
		idt_install_interrupt(_idt, 27, (uintptr_t)asm_isr27);
		idt_install_interrupt(_idt, 28, (uintptr_t)asm_isr28);
		idt_install_interrupt(_idt, 29, (uintptr_t)asm_isr29);
		idt_install_interrupt(_idt, 30, (uintptr_t)asm_isr30);
		idt_install_interrupt(_idt, 31, (uintptr_t)asm_isr31);
	}

	{ //Hardware Interrupts
		static const uint32_t IRQ_PIT = 0x20; 
		static const uint32_t IRQ_KEYBOARD = 0x21;
		extern void asm_irq0(void);
		extern void asm_irq1(void);
    extern void asm_irq2(void);
    extern void asm_irq128(void);

    extern void asm_syscall_handler(void);
    extern void asm_spurious_interrupt_handler(void);

		_interrupt_handlers[0] = (uintptr_t)irq_handler_pit;
		_interrupt_handlers[1] = (uintptr_t)irq_handler_keyboard;
    _interrupt_handlers[2] = (uintptr_t)lapic_timer_interrupt;
		idt_install_interrupt(_idt, IRQ_PIT, (uintptr_t)asm_irq0);
		idt_install_interrupt(_idt, IRQ_KEYBOARD, (uintptr_t)asm_irq1);
    idt_encode_entry((uintptr_t)&_idt[0x22], (uintptr_t)asm_irq2, true);
    idt_encode_entry((uintptr_t)&_idt[0x80], (uintptr_t)asm_syscall_handler, true);
    idt_encode_entry((uintptr_t)&_idt[0x31], (uintptr_t)asm_spurious_interrupt_handler, true);
	}
}

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
        memory_usable_range_add(mmap_entry->addr, mmap_entry->len);
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


extern void kernel_longmode_entry(uint64_t multiboot2_magic, uint64_t multiboot2_physical_address) {
	serial_debug_init();
  //NOTE(Torin 2016-09-02) At this point the kernel has been called into by our
  //bootstrap assembly and interrupts are disabled.  A Longmode GDT has been loaded
  //but the IDT still need to be configured

  remap_and_disable_legacy_pic();


  {
    idt_install_all_interrupts();
    struct {
      uint16_t limit;
      uintptr_t address;
    } __attribute__((packed)) idtr = { sizeof(_idt) - 1, (uintptr_t)_idt };
    asm volatile ("lidt %0" : : "m"(idtr));
    asm volatile ("sti");
  }

  //NOTE(Torin, 2017-10-05) This is a wierd way to get info out of the multiboot bootloader. Not
  //too thriled about how this works for now...
  //This also currently has side-effects! It initializes memory ranges in the memory manager!
  //This is too confusing @refactor
  Primary_CPU_Initialization_Info initialization_info = {};
  extract_initialization_info_from_multiboot2_tags(multiboot2_magic, 
    multiboot2_physical_address, &initialization_info);

  //NOTE(Torin, 2017-10-05) The multiboot info just provided us with memory ranges for
  //the memory manager to use so we can now initialize it.
  memory_manager_initialize();


  shell_initialize(&globals.shell);

  //NOTE(Torin, 2017-10-05) The kernel is now going to get system information from
  //the root system descriptor table.
  //TODO(Torin 2017-08-13) Make sure all RSDT data is valid before use
  if (initialization_info.rsdp_physical_address == 0) {
    klog_error("Invalid Initialization_Info: RSDP phsyical address was not found");
    kernel_panic();
  }

  //TODO(Torin, 2017-10-05) It is very confusing to be casting a physical address to a pointer
  //and relying on it being in the memory mapped section of ram. We need to check to see if it's
  //mapped already and have the parse_rsdp procedure just take a integer physical_address
  if (initialization_info.rsdp_version == 1) {
    acpi_parse_root_system_descriptor_version1((RSDP_Descriptor_1*)initialization_info.rsdp_physical_address);
  } else if (initialization_info.rsdp_version == 2) {
    acpi_parse_root_system_descriptor_version2((RSDP_Descriptor_2*)initialization_info.rsdp_physical_address);
  } else {
    klog_error("Invalid Initialization_Info: RSDP version is not supported!");
    kernel_panic();
  }

  System_Info *system = &globals.system_info;
  system->run_mode = System_Run_Mode_DEBUG_SHELL;

  kassert(system->lapic_physical_address != 0);
  kassert(system->ioapic_physical_address != 0);
  system->lapic_virtual_address = memory_map_physical_mmio(system->lapic_physical_address, 1);
  system->ioapic_virtual_address = memory_map_physical_mmio(system->ioapic_physical_address, 1);

  lapic_initialize(system->lapic_virtual_address);

  _interrupt_handlers[2] = (uintptr_t)lapic_periodic_timer_interrupt_handler;
  lapic_write_register(system->lapic_virtual_address, LAPIC_TIMER_INITAL_COUNT_REGISTER, 0xFFFF);
  lapic_write_register(system->lapic_virtual_address, LAPIC_TIMER_IRQ_NUMBER_REGISTER, 0x22 | LAPIC_TIMER_PERODIC_MODE); 
  ioapic_initalize(system->ioapic_virtual_address);
  _interrupt_handlers[2] = (uintptr_t)lapic_timer_interrupt; 

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
