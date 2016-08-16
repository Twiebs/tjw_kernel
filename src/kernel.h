#define export extern
#define external extern
#define internal static 
#define global_variable static 

#define kassert(expr) if(!(expr)) { klog_error("ASSERTION FAILED(%s) on line %u of file %s", #expr, (uint32_t)(__LINE__), __FILE__); kpanic(); }

#define KLOG_ERROR 0
#define KLOG_INFO  1
#define KLOG_DEBUG 2

#define KLOG_VERBOSITY KLOG_DEBUG

#if KLOG_VERBOSITY >= KLOG_DEBUG
#define klog_debug(...) klog_write_fmt(&globals.log, __VA_ARGS__)
#else//KLOG_VERBOSITY >= KLOG_DEBUG
#define klog_debug(...)
#endif//KLOG_VERBOSITY >= KLOG_DEBUG
#define klog_warning(...) klog_write_fmt(&globals.log, __VA_ARGS__)
#define klog_info(...)    klog_write_fmt(&globals.log, __VA_ARGS__)
#define klog_error(...)   klog_write_fmt(&globals.log, __VA_ARGS__)

#define kpanic() redraw_log_if_dirty(&globals.log); \
	asm volatile ("cli"); \
	asm volatile ("hlt")


static inline 
void write_port_uint8(uint16_t port, uint8_t value) {
	asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline
void write_port_uint32(uint16_t port, uint32_t value){
  asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}


static inline
uint8_t read_port_uint8(uint16_t port) {
	uint8_t result;
	asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
	return result;
}

static inline
uint32_t read_port_uint32(uint16_t port){
	uint32_t result;
	asm volatile ("inl %1, %0" : "=a"(result) : "Nd"(port));
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
  uintptr_t lapic_physical_address;
  uintptr_t ioapic_physical_address;
  uintptr_t lapic_virtual_address;
  uintptr_t ioapic_virtual_address;

  uint32_t processor_count;

#if 0
  uintptr_t framebuffer_address;
  uint32_t framebuffer_width;
  uint32_t framebuffer_height;
  uint8_t framebuffer_depth;
  uintptr_t framebuffer_pitch;
#endif
} System_Info;

