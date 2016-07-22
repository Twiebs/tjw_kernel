
struct idt_entry_struct {
  uint16_t offset_0_15; 
  uint16_t code_segment_selector; 
  uint8_t null_byte; 
  uint8_t type_attributes;
  uint16_t offset_16_31; 
} __attribute__((packed));


#define IDT_ENTRY_COUNT 256
global_variable idt_entry_struct idt_entries[IDT_ENTRY_COUNT];
global_variable gdt_entry_struct _gdt[3]; 




external void set_segmentation_registers(void);

internal void
x86_gdt_initialize(void) 
{
	auto gdt_set_entry = [](uint32_t n, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
		_gdt[n].base_0_15 = (base & 0xFFFF);
		_gdt[n].base_16_23 = (base >> 16) && 0xFF;
		_gdt[n].base_24_31 = (base >> 24) && 0xFF;

		_gdt[n].limit_0_15 = (limit & 0xFFFF);
		_gdt[n].granularity = (limit >> 16) & 0x0F;
		
		_gdt[n].granularity |= gran & 0xF0;
		_gdt[n].access = access;
	};


	gdt_set_entry(0, 0, 0, 0, 0); //0x0
	gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); //0x08 protected_32_code_segment
	gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); //0x10 protected_32_data_segment
#if 0
	gdt_set_entry(3, 0, 0xFFFFF); //0x18 protected_16_code_segment 
	gdt_set_entry(4, 0, 0xFFFFF); //0x20 protected_16_data_segment
	gdt_set_entry(5, 0, 0xFFFFF); //0x28 real_16_code_segment
	gdt_set_entry(6, 0, 0xFFFFF); //0x30 real_16_data_segment
#endif

#if 0 //userspace entries
	gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); //0x18
	gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); //0x20
#endif

	lgdt(_gdt, sizeof(_gdt));
	set_segmentation_registers();
	klog("GDT Inititalized");
}


extern "C" {
internal void
x86_idt_initalize() 
{

	static auto idt_install_interrupt = [](const uint32_t irq_number, const uintptr_t irq_handler_addr) 
	{
		static const uint8_t INTERRUPT_GATE_32 = 0x8E;
		static const uint8_t GDT_CODE_SEGMENT_OFFSET = 0x08;

		idt_entries[irq_number].offset_0_15 = (uint16_t)(irq_handler_addr & 0xFFFF);
		idt_entries[irq_number].offset_16_31 = (uint16_t)((irq_handler_addr >> 16) & 0xFFFF);
		idt_entries[irq_number].code_segment_selector = GDT_CODE_SEGMENT_OFFSET;
		idt_entries[irq_number].null_byte = 0;
		idt_entries[irq_number].type_attributes = INTERRUPT_GATE_32; 
	};



	//Clear and initialize the idt to an unhandled stub for debugging
	memset(idt_entries, 0, sizeof(idt_entries));
	extern void asm_irq_unhandled_stub(void);
	for (uint32_t i = 0; i < 256; i++) {
		idt_install_interrupt(i, (uintptr_t)asm_irq_unhandled_stub);
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
		extern void asm_isr8(void);
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
		extern void asm_isr32(void);

		idt_install_interrupt(0, (uintptr_t)asm_isr0);
		idt_install_interrupt(1, (uintptr_t)asm_isr1);
		idt_install_interrupt(2, (uintptr_t)asm_isr2);
		idt_install_interrupt(3, (uintptr_t)asm_isr3);
		idt_install_interrupt(4, (uintptr_t)asm_isr4);
		idt_install_interrupt(5, (uintptr_t)asm_isr5);
		idt_install_interrupt(6, (uintptr_t)asm_isr6);
		idt_install_interrupt(7, (uintptr_t)asm_isr7);
		idt_install_interrupt(8, (uintptr_t)asm_isr8);
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
		idt_install_interrupt(32, (uintptr_t)asm_isr32);
	}

	{ //Hardware Interrupts
		static const uint32_t IRQ_PIT = 0x20; 
		static const uint32_t IRQ_KEYBOARD = 0x21;
	
		extern void asm_irq0(void);
		extern void asm_irq1(void);

		interrupt_handlers[0] = irq_handler_pit;
		interrupt_handlers[1] = irq_handler_keyboard;
		idt_install_interrupt(IRQ_PIT, (uintptr_t)asm_irq0);
		idt_install_interrupt(IRQ_KEYBOARD, (uintptr_t)asm_irq1);
	}

	lidt(idt_entries, sizeof(idt_entries));
  klog("IDT initialized");
	asm volatile("sti");
}

}


#if 0
export
void kernel_entry(MultibootInfo *mbinfo) {
	klog("[GRUB Info]");
	klog("grub module count: %u", mbinfo->mods_count);
	if (mbinfo->flags & (1 << 11)) {
		klog("VBE GRUB Info availible");
		if (mbinfo->vbe_interface_seg != 0) {
			klog ("VBE protected mode interface is availible");
		} 
		if (mbinfo->vbe_interface_len != 0) {
			klog("VBE interface_len is not zero");
		}

		if (mbinfo->vbe_interface_off != 0) {
			klog("VBE interface_off is not zero");
		}
	}
	klog("==========================================");

	uint32_t *kfs_location = (uint32_t *)*((uint32_t *)(mbinfo->mods_addr));
	uint32_t *kfs_end = (uint32_t *)*((uint32_t *)(mbinfo->mods_addr + 4));
	KFS_Header *kfs = (KFS_Header *)kfs_location;
	bool is_kfs_valid = (kfs->verifier == KFS_HEADER_VERIFIER);
	klog("KFS Status: %s", is_kfs_valid ? "VALID" : "INVALID");
	if (is_kfs_valid) {
		klog("KFS File Count: %u", kfs->node_count);
	}

	_fs.kfs = kfs;
	_fs.kfs_nodes = (KFS_Node *)(kfs + 1);
	_fs.base_data = (uint8_t *)(_fs.kfs_nodes + kfs->node_count);

	kterm_clear_screen();

	safemode_checks();

	klog("[Kernel Initalization]");	
	x86_gdt_initialize();
	x86_pic8259_initalize();
	x86_idt_initalize();
	x86_pit_initialize();
	//kmem_initialize();
	klog("[TwiebsOS] v0.7 Initialized");
	klog("======================================");

	VideoState video = {};
	video_initalize(&video, mbinfo);
#if 0
	uint8_t *write = video.framebuffer;
	uint32_t color_selector = 0;
	for(uint32_t y = 0; y < video.resolutionY; y++) {
		if (color_selector == 0 && y > video.resolutionY / 3) {
			color_selector++;
		}

		if (color_selector == 1 && y > video.resolutionY / 3 * 2) {
			color_selector++;
		}

		for (uint32_t x = 0; x < video.resolutionX; x++) {
			uint32_t selector = color_selector;
			if (x > video.resolutionX / 2 && color_selector == 2) {
				selector = 0;
			}

			write[0] = selector == 0 ? 255 : 0;
			write[1] = selector == 1 ? 255 : 0;
			write[2] = selector == 2 ? 255 : 0;
			write[3] = 255;
			write += 4;
		}
	}
#endif

	//video_fill_rect(&video, video.resolutionX / 2 - 32, video.resolutionY / 2 - 32, 64, 64);
	//
		

	KFS_Node *image = kfs_find_file_with_cstr_name("out.kgi");
	uint8_t *pixels = kfs_get_node_data(image) + 16;
	memcpy(video.framebuffer, pixels, 1024*768*4);


	
#if 0	

	uint32_t x = 25, y = 25;

	uint8_t *write = video.framebuffer + (y * video.resolutionX) * video.depth;
	for (uint32_t yi = 0; yi < 50; yi++) {
		write += (video.resolutionX * video.depth);
		for (uint32_t xi = 0; xi < 50; xi++) {
			write[0] = 0;
			write[1] = 0;
			write[2] = 255;
			write[3] = 255;
			write += 4;
		}
	}
#endif
	//video_fill_rect(&video, 25, 25, 25, 25);



#if 0

	uint32_t *ptr = (uint32_t *)0xFFFFFFFF;
	*ptr = 7;

	uint32_t *ptrB = (uint32_t *)0;
	*ptrB = 6;

	uint32_t *ptrD = (uint32_t *)(0x1000 * 3);
	*ptrD = 200;
	uint32_t fault_me = *ptrD;

	uint32_t *this_will_work = (uint32_t *)(&_first_page_table + sizeof(PageTable) + 4);
	*this_will_work = 100;

	uint32_t *ptrC = (uint32_t *)0xA0000000;
	uint32_t page_fault = *ptrC;
#endif

	while (1) { asm volatile("hlt"); };
}
#endif

