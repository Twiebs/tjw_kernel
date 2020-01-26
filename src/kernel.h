#define strict_assert(expr) kassert(expr)
#define static_assert(expr) _Static_assert(expr, "ASSERTION FAILED(" #expr ")")
#define kassert(expr) if(!(expr)) { klog_error("ASSERTION FAILED(%s) on line %u of file %s", #expr, (uint32_t)(__LINE__), __FILE__); kernel_panic(); }


#define wait_for_condition(x, timeout) { \
  globals.lapic_timer_ticks = 0; \
  while((!(x)) && (globals.lapic_timer_ticks < timeout)) { asm volatile("nop"); } \
  if(globals.lapic_timer_ticks >= timeout) { klog_debug("wait timed out: %s,  %s:%u", #x, __FILE__, __LINE__); }} \
  if(globals.lapic_timer_ticks >= timeout)

void kernel_panic();



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