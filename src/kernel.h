#define export extern
#define external extern
#define internal static 
#define global_variable static 

#define strict_assert(expr) kassert(expr)
#define static_assert(expr) _Static_assert(expr, "ASSERTION FAILED(" #expr ")")
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


#define wait_for_condition(x, timeout) { \
  globals.lapic_timer_ticks = 0; \
  while((!(x)) && (globals.lapic_timer_ticks < timeout)) { asm volatile("nop"); } \
  if(globals.lapic_timer_ticks >= timeout) { klog_debug("wait timed out: %s,  %s:%u", #x, __FILE__, __LINE__); }} \
  if(globals.lapic_timer_ticks >= timeout)


#define kpanic() kgfx_draw_log_if_dirty(&globals.log); \
	asm volatile ("cli"); \
	asm volatile ("hlt")

static inline 
void write_port_uint8(uint16_t port, uint8_t value) {
	asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline
void write_port_uint32(volatile uint16_t port, volatile uint32_t value){
  asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}


static inline
uint8_t read_port_uint8(uint16_t port) {
	uint8_t result;
	asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
	return result;
}

static inline
uint16_t read_port_uint16(uint16_t port){
  uint16_t result;
  asm volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

static inline
uint32_t read_port_uint32(uint16_t port){
	uint32_t result;
	asm volatile ("inl %1, %0" : "=a"(result) : "Nd"(port));
	return result;
}

static inline 
void cpuid(int code, uint32_t *a, uint32_t *d) {
  asm volatile("cpuid":"=a"(*a),"=d"(*d):"a"(code):"ecx","ebx");
}

static inline 
void cpu_get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi) {
   asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

//TODO(Torin) Proper rebooting with ACPI
static void
kernel_reboot() {
	struct {
			uint16_t size;
			uintptr_t address;
	} __attribute__((packed)) idtr = { 0, 0 };
	asm volatile ("lidt %0" : : "m"(idtr));
	asm volatile ("int $0x3");
}

#define bochs_magic_breakpoint asm volatile("xchgw %bx, %bx")

#define SYSTEM_MAX_CPU_COUNT 16

typedef struct {
  uintptr_t lapic_physical_address;
  uintptr_t ioapic_physical_address;
  uintptr_t lapic_virtual_address;
  uintptr_t ioapic_virtual_address;

  //TODO(Torin 2016-08-29) Set for each core
  uintptr_t kernel_stack_address;

  uint32_t cpu_lapic_ids[SYSTEM_MAX_CPU_COUNT];
  uint32_t cpu_count;
  uint32_t running_cpu_count;
  Spin_Lock smp_lock;
} System_Info;

typedef struct {
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rbp;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
} Register_State;