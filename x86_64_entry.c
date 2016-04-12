// X86_64 Kernel Entry Point
// This x86_64_kernel_entry is called after
// the computer has been booted with a multiboot2 compliant
// bootloader such as GRUB2 and the bootstrap assembly has put the
// CPU into longmode with a longmode GDT; however, an longmode
// IDT still must be initalized

typedef struct {
	KFS_Header *kfs;
	KFS_Node *kfs_nodes;
	uint8_t *base_data;
} FileSystem;

typedef struct {
	uint8_t *framebuffer;
	uint16_t resolutionX;
	uint16_t resolutionY;
	uint16_t depth;
} VideoState;

typedef struct {
	uint16_t offset_0_15;
	uint16_t code_segment_selector;
	uint8_t ist;
	uint8_t type_and_attributes;
	uint16_t offset_16_31;
	uint32_t offset_32_63;
	uint32_t null_uint32;
} x86_64_IDT_Entry;

global_variable x86_64_IDT_Entry _idt[256];
global_variable uintptr_t _interrupt_handlers[256];
global_variable IOState _iostate;
global_variable FileSystem _fs;
global_variable VGATextTerm _kterm;

//=================================================================

internal void
apic_initalize(void) 
{
	asm volatile ("cli");

	{ //Disable the oldschool PIC first
		static const uint8_t PIC1_DATA_PORT = 0x21;
		static const uint8_t PIC2_DATA_PORT = 0xA1;
		write_port(PIC1_DATA_PORT, 0x20);
		write_port(PIC2_DATA_PORT, 0x20);
		write_port(PIC1_DATA_PORT, 0b11111111);
		write_port(PIC2_DATA_PORT, 0b11111111);
	}

	{ //Set the APIC SIVR

		//TODO(Torin) Get this address from the APIC base address reigster MSR
		//MSR 0x1B
		static const uint64_t APIC_REGISTER_BASE = 0xFEC00000;
		static const uint64_t APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER_OFFSET = 0xF0;

		//Interrupt Command Register
		static const uint64_t APIC_ICR1_OFFSET = 0x300;
		static const uint64_t APIC_ICR2_OFFSET = 0x310;

		uint32_t *sivr = (uint32_t *)(APIC_REGISTER_BASE + APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER_OFFSET);
		*sivr = 0xFF;
	}

	asm volatile("sti");
}


internal void
x86_pic8259_initalize(void)
{ //@Initalize And Remap the @8259_PIC @PIC @pic
	static const uint8_t PIC1_COMMAND_PORT = 0x20;
	static const uint8_t PIC2_COMMAND_PORT = 0xA0;
	static const uint8_t PIC1_DATA_PORT = 0x21;
	static const uint8_t PIC2_DATA_PORT = 0xA1;

	static const uint8_t ICW1_INIT_CASCADED = 0x11;
	static const uint8_t ICW2_PIC1_IRQ_NUMBER_BEGIN = 0x20;
	static const uint8_t ICW2_PIC2_IRQ_NUMBER_BEGIN = 0x28;
	static const uint8_t ICW3_PIC1_IRQ_LINE_2 = 0x4;
	static const uint8_t ICW3_PIC2_IRQ_LINE_2 = 0x2;
	static const uint8_t ICW4_8068 = 0x01;

	//Initalization Command Words (ICW)
	//ICW1 Tells PIC to wait for 3 more words
	write_port(PIC1_COMMAND_PORT, ICW1_INIT_CASCADED);
	write_port(PIC2_COMMAND_PORT, ICW1_INIT_CASCADED);
	//ICW2 Set PIC Offset Values
	write_port(PIC1_DATA_PORT, ICW2_PIC1_IRQ_NUMBER_BEGIN);
	write_port(PIC2_DATA_PORT, ICW2_PIC2_IRQ_NUMBER_BEGIN);
	//ICW3 PIC Cascading Info
	write_port(PIC1_DATA_PORT, ICW3_PIC1_IRQ_LINE_2);
	write_port(PIC2_DATA_PORT, ICW3_PIC2_IRQ_LINE_2);
	//ICW4 Additional Enviroment Info
	//NOTE(Torin) Currently set to 80x86
	write_port(PIC1_DATA_PORT, ICW4_8068);
	write_port(PIC2_DATA_PORT, ICW4_8068);
	
	write_port(PIC1_DATA_PORT, 0x20);
	write_port(PIC2_DATA_PORT, 0x20);
	write_port(PIC1_DATA_PORT, 0b11111101);
	write_port(PIC2_DATA_PORT, 0b11111111);
	klog("PIC8259 Initialized and Remaped");
}

	internal void
x86_pit_initialize(void)  
{ //@Initialize @pit @PIT
	//TODO(Torin) This frequency thing is probably bogus and needs to be
	//fixed so that specific times can be programmed
	static const uint32_t PIT_COMMAND_REPEATING_MODE = 0x36;
	static const uint32_t PIT_DIVISOR = 1193180 / 100;

	static const uint8_t PIT_DATA_PORT0 = 0x40;
	static const uint8_t PIT_DATA_PORT1 = 0x41;
	static const uint8_t PIT_DATA_PORT2 = 0x42;
	static const uint8_t PIT_COMMAND_PORT = 0x43;

	const uint8_t divisor_low = (uint8_t)(PIT_DIVISOR & 0xFF);
	const uint8_t divisor_high = (uint8_t)((PIT_DIVISOR >> 8) & 0xFF);

	write_port(PIT_COMMAND_PORT, PIT_COMMAND_REPEATING_MODE);
	write_port(PIT_DATA_PORT0, divisor_low);
	write_port(PIT_DATA_PORT0, divisor_high);
	klog ("PIT initialized!");
}

internal void 
vga_text_clear_screen() {  
	static const uint8_t VGA_TEXT_COLUMN_COUNT = 80;
	static const uint8_t VGA_TEXT_ROW_COUNT = 25;
	uint8_t *VGA_TEXT_BUFFER = (uint8_t*)(0xB8000);
	memset(VGA_TEXT_BUFFER, 0, 
			(VGA_TEXT_COLUMN_COUNT * VGA_TEXT_ROW_COUNT) * 2);
}

internal void
idt_install_interrupt(const uint32_t irq_number, const uint64_t irq_handler_addr) 
{
	static const uint64_t PRIVILEGE_LEVEL_0 = 0b00000000;
	static const uint64_t PRIVILEGE_LEVEL_3 = 0b01100000;
	static const uint64_t PRESENT_BIT = (1 << 7); 
	//While running in long mode these flags are redefined to be 64bits
	//NOTE(Torin) See AMD64 VOL2 247
	static const uint64_t TYPE_TASK_GATE_32 = 0x5;
	static const uint64_t TYPE_INTERRUPT_GATE_32 = 0xE;
	static const uint64_t TYPE_TRAP_GATE_32 = 0xF;
	//Offset setup in protected mode assembly before the switch to long mode
	static const uint8_t GDT_CODE_SEGMENT_OFFSET = 0x08;
	
	_idt[irq_number].offset_0_15 = (uint16_t)(irq_handler_addr & 0xFFFF);
	_idt[irq_number].offset_16_31 = (uint16_t)((irq_handler_addr >> 16) & 0xFFFF);
	_idt[irq_number].offset_32_63 = (uint32_t)((irq_handler_addr >> 32) & 0xFFFFFFFF);
	_idt[irq_number].type_and_attributes = PRESENT_BIT | TYPE_INTERRUPT_GATE_32 | PRIVILEGE_LEVEL_0; 
	_idt[irq_number].code_segment_selector = GDT_CODE_SEGMENT_OFFSET;
	_idt[irq_number].ist = 0;
}

extern void asm_double_fault_handler();
extern void asm_debug_handler();

internal void
x86_64_idt_initalize() 
{


	for (uint32_t i = 0; i < 256; i++) {
		idt_install_interrupt(i, (uintptr_t)asm_debug_handler);
		_interrupt_handlers[i] = 0xFFFFFFFF;
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

#if 1
	{ //Hardware Interrupts
		static const uint32_t IRQ_PIT = 0x20; 
		static const uint32_t IRQ_KEYBOARD = 0x21;
		extern void asm_irq0(void);
		extern void asm_irq1(void);

		_interrupt_handlers[0] = (uintptr_t)irq_handler_pit;
		_interrupt_handlers[1] = (uintptr_t)irq_handler_keyboard;
		idt_install_interrupt(IRQ_PIT, (uintptr_t)asm_irq0);
		idt_install_interrupt(IRQ_KEYBOARD, (uintptr_t)asm_irq1);
	}
#endif

	struct {
			uint16_t limit;
			uintptr_t address;
	} __attribute__((packed)) idtr = { sizeof(_idt) - 1, (uintptr_t)_idt };
	asm volatile ("lidt %0" : : "m"(idtr));
	asm volatile ("sti");
  klog("IDT initialized");
}

extern uintptr_t p4_table;
extern uintptr_t p3_table;
extern uintptr_t p2_table;


export void 
kernel_longmode_entry() {
	x86_pic8259_initalize();
	x86_pit_initialize();
	x86_64_idt_initalize();
	

	klog("holy fucking shit balls!  is this actualy working finaly?");
	//apic_initalize();


	while (1) { 
		kterm_redraw_if_required(&_kterm, &_iostate);
		asm volatile("hlt"); 
	};


#if 0	
	uint32_t *module_addr_array = (uint32_t *)((uintptr_t)mb->mods_addr);
	KFS_Header *kfs = (KFS_Header *)(uintptr_t)module_addr_array[0];
	bool is_kfs_valid = (kfs->verifier == KFS_HEADER_VERIFIER);
	klog("KFS Status: %s", is_kfs_valid ? "VALID" : "INVALID");
	if (is_kfs_valid) {
		klog("KFS File Count: %u", kfs->node_count);
	}

	_fs.kfs = kfs;
	_fs.kfs_nodes = (KFS_Node *)(kfs + 1);
	_fs.base_data = (uint8_t *)(_fs.kfs_nodes + kfs->node_count);
#endif



}
