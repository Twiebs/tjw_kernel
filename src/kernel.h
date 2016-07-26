#define export extern
#define external extern
#define internal static 
#define global_variable static 

#define kassert(expr) if(!(expr)) { klog_error("ASSERTION FAILED!!!"); kpanic(); }

#define klog_debug(...) console_write_fmt(&globals.console_buffer, __VA_ARGS__)
#define klog_info(...) console_write_fmt(&globals.console_buffer, __VA_ARGS__)
#define klog_error(...) console_write_fmt(&globals.console_buffer, __VA_ARGS__)

#define kpanic() redraw_vga_text_terminal_if_dirty(&globals.vga_text_term, &globals.console_buffer); \
	asm volatile ("cli"); \
	asm volatile ("hlt")

#if 0
#define COM1_PORT 0x3F8
#define COM2_PORT 0x2F8
#define COM3_PORT 0x3E8
#define COM4_PORT 0x2E8
#endif

//TODO(Torin) Move this out to kernel_io ?

internal inline 
void write_port(uint16_t port, uint8_t value) {
	asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

internal inline
uint8_t read_port(uint16_t port) {
	uint8_t result;
	asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
	return result;
}

//TODO(Torin) Proper rebooting with ACPI
internal void
kernel_reboot() {
	struct {
			uint16_t size;
			uintptr_t address;
	} __attribute__((packed)) idtr = { 0, 0 };
	asm volatile ("lidt %0" : : "m"(idtr));
	asm volatile ("int $0x3");
}



typedef struct {
  uintptr_t ioapic_register_base;
  uintptr_t lapic_register_base;
  uint32_t processor_count;
} System_Info;

