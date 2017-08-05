// NOTE(Torin)
// X86_64 Kernel Entry Point
// This x86_64_kernel_entry is called after
// the computer has been booted with a multiboot2 compliant
// bootloader such as GRUB2 and the bootstrap assembly has put the
// CPU into longmode with a longmode GDT; however, an longmode
// IDT still must be initalized


/*
 
 ** Current Feature TODO List **
- PCIe MMIO registers
- MSI interrupts 
- Create os specific bootloader
- AHCI(SATA) Driver
- XHCI Driver
- OCHI Driver
- UHCI Driver
- Switch to real mode and properly change video modes
*/



typedef struct {
  Circular_Log log;
  Command_Line_Shell shell;
  //VGA_Text_Terminal vga_text_term;
  Keyboard_State keyboard;
  Kernel_Memory_State memory_state;
  System_Info system_info;
  Framebuffer framebuffer;
  Task_Info task_info;
  Ext2_Filesystem ext2_filesystem;


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
  asm volatile ("cli");
  asm volatile ("hlt");
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


uint32_t get_cpu_id(){
  System_Info *system = &globals.system_info;
  uint32_t lapic_id = lapic_get_id(globals.system_info.lapic_virtual_address);
  for (size_t i = 0; i < system->total_cpu_count; i++) {
    if (system->cpu_lapic_ids[i] == lapic_id) { return i; }
  }

  klog_error("UNREGISTER LAPIC ID WAS USED TO GFT CPU ID");
  kernel_panic();
  return 0;
}

#include "kernel_acpi.c"
//#include "kernel_descriptor.c"
#include "descriptor_tables.c"
#include "kernel_exceptions.c"
#include "kernel_task.c"
#include "kernel_pci.c"
#include "kernel_memory.c"
#include "kernel_debug.c"
#include "hardware_keyboard.c"



#include "usb/usb_protocol.c"
#include "usb/ehci.c"

static IDT_Entry _idt[256];
static uintptr_t _interrupt_handlers[256];

static const uint8_t TRAMPOLINE_BINARY[] = {
#include "trampoline.txt"
};

#if 1
static const uint8_t TEST_PROGRAM_ELF[] = {
#include "test_program.txt" 
};
#endif


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

extern void
ap_entry_procedure(void){
  asm volatile("hlt");
}



void initalize_task_state_segment(CPU_Info *cpu_info) {
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
  cpu_info->kernel_stack_top = stack_top;
  initalize_task_state_segment(cpu_info);


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


extern void kernel_longmode_entry(uint64_t multiboot2_magic, uint64_t multiboot2_address) {
	serial_debug_init();
  //NOTE(Torin 2016-09-02) At this point the kernel has been called into by our
  //bootstrap assembly and interrupts are disabled.  A Longmode GDT has been loaded
  //but the IDT still need to be configured

  { //NOTE(Torin) Remap the legacy PIC8259 and mask out the interrupt vectors 
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
    write_port_uint8(PIC1_DATA_PORT, 0b11111111);
    write_port_uint8(PIC2_DATA_PORT, 0b11111111);
  }

  {
    idt_install_all_interrupts();
    struct {
      uint16_t limit;
      uintptr_t address;
    } __attribute__((packed)) idtr = { sizeof(_idt) - 1, (uintptr_t)_idt };
    asm volatile ("lidt %0" : : "m"(idtr));
    asm volatile ("sti");
  }


  //log_disable(DEBUG0);

	if (multiboot2_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		klog_error("the kernel was not booted with a multiboot2 compliant bootloader!");
		kernel_panic();
	}

	if (multiboot2_address & 7) {
		klog_error("unaligned multiboot_info!");
		kernel_panic();
	}

  uintptr_t rsdp_physical_address = 0;
  struct multiboot_tag_framebuffer *fb_mbtag = 0; 
  struct multiboot_tag_mmap *mmap_tag = 0;

  { //NOTE(Torin) Extract relevant multiboot information
  	struct multiboot_tag *tag = (struct multiboot_tag *)(multiboot2_address + 8);
  	while (tag->type != MULTIBOOT_TAG_TYPE_END) {
  		switch (tag->type) {
  			case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
  		    struct multiboot_tag_old_acpi *acpi_info = (struct multiboot_tag_old_acpi *)(tag);
          rsdp_physical_address = (uintptr_t)acpi_info->rsdp;
  			} break;
  			case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
  				struct multiboot_tag_new_acpi *acpi_info = (struct multiboot_tag_new_acpi *)(tag);
          rsdp_physical_address = (uintptr_t)acpi_info->rsdp;
  			} break;

        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: fb_mbtag = (struct multiboot_tag_framebuffer *)(tag); break;
        case MULTIBOOT_TAG_TYPE_MMAP: mmap_tag = (struct multiboot_tag_mmap *)(tag); break;

  		}
  		tag = (struct multiboot_tag *)(((uint8_t *)tag) + ((tag->size + 7) & ~7));
  	}
  } 

  

  if(mmap_tag != 0){
    multiboot_memory_map_t *mmap_entry = (multiboot_memory_map_t *)(mmap_tag->entries);
    while((uintptr_t)mmap_entry < (uintptr_t)mmap_tag + mmap_tag->size){
      static const uint8_t MEMORY_AVAILABLE = 1;
      if(mmap_entry->type == MEMORY_AVAILABLE && mmap_entry->addr >= 0x100000){
        kmem_add_usable_range(mmap_entry->addr, mmap_entry->len, &globals.memory_state);
      }
       mmap_entry = (multiboot_memory_map_t *)((uintptr_t)mmap_entry + mmap_tag->entry_size);
    }
  }

  kmem_initalize_memory_state(&globals.memory_state);

  //NOTE(Torin) Initalize the framebuffer
  if(fb_mbtag == 0) {
    kassert(0 && "MULTIBOOT FAILED TO PROVIDE FRAMEBUFFER TAG");
  }

  if(fb_mbtag->common.framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB){
    //NOTE(Torin) Text buffer
  } else {

    return;
    //TODO(TORIN 2016-10-20) Framebuffer currently broken

 #if 0 
    uintptr_t framebuffer_virtual_address = 0x0A000000;
    uintptr_t page_offset = kmem_map_unaligned_physical_to_aligned_virtual_2MB(fb_mbtag->common.framebuffer_addr, framebuffer_virtual_address, PAGE_USER_ACCESS_BIT);
    kmem_map_physical_to_virtual_2MB_ext(fb_mbtag->common.framebuffer_addr - page_offset + 0x200000, framebuffer_virtual_address + 0x200000, PAGE_USER_ACCESS_BIT);

    Framebuffer *fb = &globals.framebuffer;
    fb->width = fb_mbtag->common.framebuffer_width; 
    fb->height = fb_mbtag->common.framebuffer_height;
    fb->buffer = (uint8_t *)framebuffer_virtual_address;
    fb->depth = fb_mbtag->common.framebuffer_bpp / 8; 
    fb->pitch = fb_mbtag->common.framebuffer_pitch; 
    klog_debug("framebuffer: width: %u, height: %u, depth: %u", fb->width, fb->height, (uint32_t)fb->depth);
  #endif
  }


  if(rsdp_physical_address == 0){
    klog_error("MULTIBOOT FAILED TO PROVIDE LOCATION OF RSDP");
    return;
  }

  System_Info *sys = &globals.system_info;
  if (parse_root_system_descriptor((RSDP_Descriptor_1*)rsdp_physical_address, sys) == 0) {
    klog_error("unabled to parse root system descriptor");
    return; 
  }


  { //NOTE(Torin) Initalize the lapic and configure the lapic timer
    //NOTE(Torin) Arbitrarly maps the lapic and ioapic into the kernels virtual addresss space
    //And initalizes the iopapic and lapic
    sys->lapic_virtual_address = kmem_map_physical_mmio(&globals.memory_state, sys->lapic_physical_address, 1);
    sys->ioapic_virtual_address = kmem_map_physical_mmio(&globals.memory_state, sys->ioapic_physical_address, 1);
    klog_debug("lapic_virtual_address: 0x%X", sys->lapic_virtual_address);
    klog_debug("ioapic_virtual_address: 0x%X", sys->ioapic_virtual_address);
    
    static const uint64_t LAPIC_SIVR_REGISTER = 0xF0;
    static const uint32_t LAPIC_SIVR_ENABLE = 1 << 8;
    static const uint32_t LAPIC_VERSION_REGISTER = 0x30;

    asm volatile("cli");
    lapic_write_register(sys->lapic_virtual_address, LAPIC_SIVR_REGISTER, 0x31 | LAPIC_SIVR_ENABLE);
    asm volatile("sti");

    uint32_t lapic_version_register = lapic_read_register(sys->lapic_virtual_address, LAPIC_VERSION_REGISTER);
    uint32_t lapic_version = lapic_version_register & 0xFF;
    uint32_t lapic_max_lvt_entries = (lapic_version_register >> 16) & 0xFF;
    klog_debug("lapic_version: %u", lapic_version);
    klog_debug("lapic_max_lvt_entries: %u", lapic_max_lvt_entries);

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

    klog_debug("initalzing lapic timer");

    { //NOTE(Torin) Configure the PIT Timer 
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

    static const uint8_t PIC1_DATA_PORT = 0x21; 
    //NOTE(Torin) Unmask the PIT timer interrupt
    write_port_uint8(PIC1_DATA_PORT, 0b11111110); 
    //NOTE(Torin) Start the LAPIC Timer
    lapic_write_register(sys->lapic_virtual_address, LAPIC_TIMER_DIVIDE_CONFIG_REGISTER, LAPIC_TIMER_DIVIDE_2);
    lapic_write_register(sys->lapic_virtual_address, LAPIC_TIMER_INITAL_COUNT_REGISTER, 0xFFFFFFFF);
    lapic_write_register(sys->lapic_virtual_address, LAPIC_TIMER_IRQ_NUMBER_REGISTER, 0x22 | LAPIC_TIMER_IRQ_MASKED);
    //NOTE(Torin) Count how many ticks the LAPIC decrements in 32 milliseconds
    globals.pit_timer_ticks = 0;
    while(globals.pit_timer_ticks <= 32){ asm volatile("nop"); }
    uint32_t ticks_remaining = lapic_read_register(sys->lapic_virtual_address, LAPIC_TIMER_CURRENT_COUNT_REGISTER);  
    uint32_t ticks_in_32_ms = 0xFFFFFFFF - ticks_remaining;
    uint32_t ticks_per_millisecond = ticks_in_32_ms / 32;
    if(ticks_per_millisecond == 0){
      klog_error("error initializing lapic timer");
      return;
    }

    klog_debug("ticks_per_millisecond: %u", ticks_per_millisecond);
    //NOTE(Torin 2016-10-15) Disable PIT timer interrupt for PIC
    write_port_uint8(PIC1_DATA_PORT, 0b11111111); 

    //NOTE(Torin) Install main lapic timer interrupt handler
    _interrupt_handlers[2] = (uintptr_t)lapic_periodic_timer_interrupt_handler;
    lapic_write_register(sys->lapic_virtual_address, LAPIC_TIMER_INITAL_COUNT_REGISTER, ticks_per_millisecond);
    lapic_write_register(sys->lapic_virtual_address, LAPIC_TIMER_IRQ_NUMBER_REGISTER, 0x22 | LAPIC_TIMER_PERODIC_MODE); 
    ioapic_initalize(sys->ioapic_virtual_address);
  }

  initalize_cpu_info_and_start_secondary_cpus(sys); 


  pci_scan_devices();

  _interrupt_handlers[2] = (uintptr_t)lapic_timer_interrupt; 
  //lapic_configure_timer(sys->lapic_virtual_address, 0xFFFF, 0x22, 1);

	while(1) { 
    shell_update(&globals.shell);
  };
}
