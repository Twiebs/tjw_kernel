// NOTE(Torin)
// X86_64 Kernel Entry Point
// This x86_64_kernel_entry is called after
// the computer has been booted with a multiboot2 compliant
// bootloader such as GRUB2 and the bootstrap assembly has put the
// CPU into longmode with a longmode GDT; however, an longmode
// IDT still must be initalized

typedef struct {
  Circular_Log log;
  VGA_Text_Terminal vga_text_term;
  Keyboard_State keyboard;

  System_Info system_info;
  Framebuffer framebuffer;

  Task_Info task_info;

  volatile uint64_t pit_timer_ticks;

  bool is_logging_disabled;
  bool log_keyboard_events;
} Kernel_Globals;


static Kernel_Globals globals;

#define GDT_RING0_DATA 0x08
#define GDT_RING3_DATA 0x10
#define GDT_RING0_CODE 0x18
#define GDT_RING3_CODE 0x20
#define GDT_TSS 0x28

#include "kernel_graphics.c"
#include "kernel_acpi.c"
#include "kernel_apic.c"
#include "kernel_descriptor.c"
#include "kernel_exceptions.c"

static IDT_Entry _idt[256];
static uintptr_t _interrupt_handlers[256];

static const uint8_t TRAMPOLINE_BINARY[] = {
#include "trampoline.txt"
};

static const uint8_t TEST_PROGRAM_ELF[] = {
#include "test_program.txt" 
};

static inline
uint32_t get_cpu_id(){
  uint32_t lapic_id = lapic_get_id(globals.system_info.lapic_virtual_address);
  for(size_t i = 0; i < globals.system_info.cpu_count; i++){
    if(globals.system_info.cpu_lapic_ids[0] == lapic_id) return i;
  }
  klog_error("UNREGISTER LAPIC ID WAS USED TO GFT CPU ID");
  kpanic();
  return 0;
}

#if 0
static uint16_t
pit_read_timer(){
  static const uint8_t PIT_CHANNEL0_DATA_PORT = 0x40;
  static const uint8_t PIT_COMMAND_PORT = 0x43;
  write_port_uint8(PIT_COMMAND_PORT, 0x00);  
  uint8_t count_low = read_port_uint8(PIT_CHANNEL0_DATA_PORT);
  uint8_t count_high = read_port_uint8(PIT_CHANNEL0_DATA_PORT);
  uint16_t count = (count_high << 8) | count_low;
  return count;
}
#endif

static void
pit_configure_timer(const uint32_t requested_frequency){
  static const uint8_t PIT_CHANNEL0_DATA_PORT = 0x40;
  static const uint8_t PIT_CHANNEL1_DATA_PORT = 0x41;
  static const uint8_t PIT_CHANNEL2_DATA_PORT = 0x42; 
  static const uint8_t PIT_COMMAND_PORT = 0x43;

  static const uint8_t COMMAND_CHANNEL0 = (0b00) << 6;
  static const uint8_t COMMAND_CHANNEL1 = (0b01) << 6;
  static const uint8_t COMMAND_CHANNEL2 = (0b11) << 6;
  static const uint8_t COMMAND_ACCESS_LOW_AND_HIGH = (0b11) << 4;
  static const uint8_t COMMAND_MODE_RATE_GENERATOR = (0b010) << 1;
  static const uint8_t COMMAND_MODE_SQUARE_WAVE = (0b011) << 1;

  static const uint32_t PIT_BASE_FREQUENCY = 1193182;

  kassert(requested_frequency > 18);
  kassert(requested_frequency < 1193181);

  uint32_t divisor = PIT_BASE_FREQUENCY / requested_frequency;
  uint8_t divisor_low = (uint8_t)(divisor & 0xFF);
  uint8_t divisor_high = (uint8_t)((divisor >> 8) & 0xFF);

  write_port_uint8(PIT_COMMAND_PORT, 0x36);
  write_port_uint8(PIT_CHANNEL0_DATA_PORT, divisor_low);
  write_port_uint8(PIT_CHANNEL0_DATA_PORT, divisor_high);
}

static void
pit_wait_milliseconds(const volatile uint32_t ms){
  globals.pit_timer_ticks = 0;
  while (globals.pit_timer_ticks < ms) { }
}

//TODO(Torin) Remove IDT Global variable
static void
idt_install_interrupt(const uint32_t irq_number, const uint64_t irq_handler_addr) {
	static const uint64_t PRIVILEGE_LEVEL_0 = 0b00000000;
	static const uint64_t PRIVILEGE_LEVEL_3 = 0b01100000;
	static const uint64_t PRESENT_BIT = (1 << 7); 
	
	static const uint64_t TYPE_TASK_GATE_64 = 0x5;
	static const uint64_t TYPE_INTERRUPT_GATE_64 = 0xE;
	static const uint64_t TYPE_TRAP_GATE_64 = 0xF;

	_idt[irq_number].offset_0_15 = (uint16_t)(irq_handler_addr & 0xFFFF);
	_idt[irq_number].offset_16_31 = (uint16_t)((irq_handler_addr >> 16) & 0xFFFF);
	_idt[irq_number].offset_32_63 = (uint32_t)((irq_handler_addr >> 32) & 0xFFFFFFFF);
	_idt[irq_number].type_and_attributes = PRESENT_BIT | TYPE_INTERRUPT_GATE_64 | PRIVILEGE_LEVEL_0; 
	_idt[irq_number].code_segment_selector = GDT_RING0_CODE;
	_idt[irq_number].ist = 0;
}

static void
idt_install_all_interrupts(){
  extern void asm_double_fault_handler();
  extern void asm_debug_handler();
  
	for (uint32_t i = 0; i < 256; i++) {
		idt_install_interrupt(i, (uintptr_t)asm_debug_handler);
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

		idt_install_interrupt(0, (uintptr_t)asm_isr0);
		idt_install_interrupt(1, (uintptr_t)asm_isr1);
		idt_install_interrupt(2, (uintptr_t)asm_isr2);
		idt_install_interrupt(3, (uintptr_t)asm_isr3);
		idt_install_interrupt(4, (uintptr_t)asm_isr4);
		idt_install_interrupt(5, (uintptr_t)asm_isr5);
		idt_install_interrupt(6, (uintptr_t)asm_isr6);
		idt_install_interrupt(7, (uintptr_t)asm_isr7);
		idt_install_interrupt(8, (uintptr_t)asm_double_fault_handler);
		idt_install_interrupt(9, (uintptr_t)asm_isr9);
		idt_install_interrupt(10, (uintptr_t)asm_isr10);
		idt_install_interrupt(11, (uintptr_t)asm_isr11);
		idt_install_interrupt(12, (uintptr_t)asm_isr12);
		idt_install_interrupt(13, (uintptr_t)asm_isr13);
		idt_install_interrupt(14, (uintptr_t)asm_isr14);
		idt_install_interrupt(15, (uintptr_t)asm_isr15);
		idt_install_interrupt(16, (uintptr_t)asm_isr16);
		idt_install_interrupt(17, (uintptr_t)asm_isr17);
		idt_install_interrupt(18, (uintptr_t)asm_isr18);
		idt_install_interrupt(19, (uintptr_t)asm_isr19);
		idt_install_interrupt(20, (uintptr_t)asm_isr20);
		idt_install_interrupt(21, (uintptr_t)asm_isr21);
		idt_install_interrupt(22, (uintptr_t)asm_isr22);
		idt_install_interrupt(23, (uintptr_t)asm_isr23);
		idt_install_interrupt(24, (uintptr_t)asm_isr24);
		idt_install_interrupt(25, (uintptr_t)asm_isr25);
		idt_install_interrupt(26, (uintptr_t)asm_isr26);
		idt_install_interrupt(27, (uintptr_t)asm_isr27);
		idt_install_interrupt(28, (uintptr_t)asm_isr28);
		idt_install_interrupt(29, (uintptr_t)asm_isr29);
		idt_install_interrupt(30, (uintptr_t)asm_isr30);
		idt_install_interrupt(31, (uintptr_t)asm_isr31);
	}

	{ //Hardware Interrupts
		static const uint32_t IRQ_PIT = 0x20; 
		static const uint32_t IRQ_KEYBOARD = 0x21;
		extern void asm_irq0(void);
		extern void asm_irq1(void);
    extern void asm_irq128(void);

    extern void asm_syscall_handler(void);
    extern void asm_spurious_interrupt_handler(void);

		_interrupt_handlers[0] = (uintptr_t)irq_handler_pit;
		_interrupt_handlers[1] = (uintptr_t)irq_handler_keyboard;
		idt_install_interrupt(IRQ_PIT, (uintptr_t)asm_irq0);
		idt_install_interrupt(IRQ_KEYBOARD, (uintptr_t)asm_irq1);
    idt_encode_entry((uintptr_t)&_idt[0x80], (uintptr_t)asm_syscall_handler, true);
    idt_encode_entry((uintptr_t)&_idt[0x31], (uintptr_t)asm_spurious_interrupt_handler, true);
	}
}

#include "multiboot2.h"
#include "hardware_serial.c"
#include "kernel_memory.c"

extern void
ap_entry_procedure(void){
  asm volatile("hlt");
}

#include "kernel_pci.h"
#include "usb_controller.h"

#include "kernel_task.c"
#include "kernel_pci.c"
#include "kernel_debug.c"
#include "usb_protocol.c"
#include "usb_ehci.c"


extern void 
kernel_longmode_entry(uint64_t multiboot2_magic, uint64_t multiboot2_address)
{
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
    //NOTE(Torin) The PIT is left unmaksed inorder to initalize the apic timer
    //after the apic timer is initalized it will be disabled
    write_port_uint8(PIC1_DATA_PORT, 0b11111110);
    write_port_uint8(PIC2_DATA_PORT, 0b11111111);
  }

  { //NOTE(Torin) Initalize the IDT
    //NOTE(Torin 2016-09-02) At some point in the future this should just be 
    //A simple load on the staticly baked IDT table that will be created with an
    //Ofline tool.  For now it is just done a runtime for simplicity
    idt_install_all_interrupts();
    struct {
      uint16_t limit;
      uintptr_t address;
    } __attribute__((packed)) idtr = { sizeof(_idt) - 1, (uintptr_t)_idt };
    asm volatile ("lidt %0" : : "m"(idtr));
    asm volatile ("sti");
  }


  kmem_initalize();

  //NOTE(Torin) Setup keyboard event stack
  globals.keyboard.scancode_event_stack = globals.keyboard.scancode_event_stack0;


	if (multiboot2_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		klog_error("the kernel was not booted with a multiboot2 compliant bootloader!");
		kpanic();
	}

	if (multiboot2_address & 7) {
		klog_error("unaligned multiboot_info!");
		kpanic();
	}

  struct multiboot_tag_framebuffer *fb_mbtag = 0; 
  uintptr_t rsdp_physical_address = 0;
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

      case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
        fb_mbtag = (struct multiboot_tag_framebuffer *)(tag);

      } break;

      case MULTIBOOT_TAG_TYPE_MMAP: {
        #if 0
        klog_debug("found mmap multiboot tag");
        struct multiboot_tag_mmap *mmap_tag = (struct multiboot_tag_mmap *)(tag);
        multiboot_memory_map_t *mmap_entry = (multiboot_memory_map_t *)(mmap_tag->entries);
        while((uintptr_t)mmap_entry < (uintptr_t)mmap_tag + mmap_tag->size){
          const char *MB_MMAP_TYPE_STRINGS[] = {
            "INVALID_TYPE",
            "MEMORY_AVAILABLE",
            "MEMORY_ACPI_RECLAIMABLE",
            "MEMORY_NON_VOLITALE_STORAGE",
            "MEMORY_BAD_RAM"
          };

          klog_debug("addr: %lu, size: %lu, type: %s", mmap_entry->addr, mmap_entry->len, MB_MMAP_TYPE_STRINGS[mmap_entry->type]);
          mmap_entry = (multiboot_memory_map_t *)((uintptr_t)mmap_entry + mmap_tag->entry_size);
        }
        #endif
      };

		}
		tag = (struct multiboot_tag *)(((uint8_t *)tag) + ((tag->size + 7) & ~7));
	}

  //NOTE(Torin) Initalize the framebuffer
  if(fb_mbtag == 0) {
    kassert(0 && "MULTIBOOT FAILED TO PROVIDE FRAMEBUFFER TAG");
  }

  if(fb_mbtag->common.framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB){
    klog_info("this is a text buffer");
  } else {
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
  }


  if(rsdp_physical_address == 0){
    kassert(0 && "MULTIBOOT FAILED TO PROVIDE LOCATION OF RSDP");
  }

  System_Info *sys = &globals.system_info;
  parse_root_system_descriptor((RSDP_Descriptor_1*)rsdp_physical_address, sys);

  {
    //TODO(Torin 2016-08-29) This should probably be established after
    //the other cpus are initalized
    extern uintptr_t stack_top;
    uintptr_t stack_top_ptr = (uintptr_t)&stack_top;
    sys->kernel_stack_address = stack_top_ptr;
    memset(&g_tss_entry, 0x00, sizeof(g_tss_entry));
    g_tss_entry.rsp0 = stack_top_ptr;
    g_tss_entry.ist1 = stack_top_ptr;
    klog_debug("tss rsp0: 0x%X", g_tss_entry.rsp0);
    uint8_t *gdt = (uint8_t *)&GDT64;
    gdt_encode_system_descriptor((uintptr_t)&g_tss_entry, 0xFF, GDT_DESCRIPTOR_TYPE_TSS, 3, (uintptr_t)(gdt + GDT_TSS));
    tss_ldr(GDT_TSS);
  }

  { //NOTE(Torin) Initalize the lapic and configure the lapic timer
    //NOTE(Torin) Arbitrarly maps the lapic and ioapic into the kernels virtual addresss space
    //And initalizes the iopapic and lapic
    sys->lapic_virtual_address = 0x0C200000;
    sys->ioapic_virtual_address = 0x0C400000;
    kmem_map_physical_to_virtual_2MB(sys->lapic_physical_address, sys->lapic_virtual_address);
    kmem_map_physical_to_virtual_2MB(sys->ioapic_physical_address, sys->ioapic_virtual_address);
    asm volatile("cli");
    static const uint64_t LAPIC_SIVR_REGISTER = 0xF0;
    static const uint32_t LAPIC_SIVR_ENABLE = 1 << 8;
    lapic_write_register(sys->lapic_virtual_address, LAPIC_SIVR_REGISTER, 0x31 | LAPIC_SIVR_ENABLE);
    asm volatile("sti");

    static const uint32_t LAPIC_TIMER_IRQ_NUMBER_REGISTER = 0x320;
    static const uint32_t LAPIC_TIMER_INITAL_COUNT_REGISTER = 0x380;
    static const uint32_t LAPIC_TIMER_CURRENT_COUNT_REGISTER = 0x390;
    static const uint32_t LAPIC_TIMER_DIVIDE_CONFIG_REGISTER = 0x3E0;

    static const uint32_t LAPIC_TIMER_IRQ_MASKED = 1 << 16;
    static const uint32_t LAPIC_TIMER_DIVIDE_2 = 0b00;
    static const uint32_t LAPIC_TIMER_DIVIDE_4 = 0b01;
    static const uint32_t LAPIC_TIMER_DIVIDE_8 = 0b10;
    static const uint32_t LAPIC_TIMER_DIVIDE_16 = 0b11;

    klog_debug("initalzing lapic timer");
    lapic_write_register(sys->lapic_virtual_address, LAPIC_TIMER_DIVIDE_CONFIG_REGISTER, LAPIC_TIMER_DIVIDE_16);
    lapic_write_register(sys->lapic_virtual_address, LAPIC_TIMER_INITAL_COUNT_REGISTER, 0xFFFFFFFF);
    lapic_write_register(sys->lapic_virtual_address, LAPIC_TIMER_IRQ_NUMBER_REGISTER, 0x22 | LAPIC_TIMER_IRQ_MASKED);
    pit_wait_milliseconds(10); 
    uint32_t ticks_remaining = lapic_read_register(sys->lapic_virtual_address, LAPIC_TIMER_CURRENT_COUNT_REGISTER);  
    uint32_t ticks_in_10_ms = 0xFFFFFFFF - ticks_remaining;
    klog_debug("lapic ticks in 10 ms: %u", ticks_in_10_ms);

    ioapic_initalize(sys->ioapic_virtual_address);
    klog_debug("apic initalized");
  }


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
  kgfx_draw_log_if_dirty(&globals.log);
  pci_scan_devices();
  kgfx_draw_log_if_dirty(&globals.log);

  //lapic_configure_timer(sys->lapic_virtual_address, 0xFFFF, 0x20, 1);

	while(1) { asm volatile("hlt"); };
}
