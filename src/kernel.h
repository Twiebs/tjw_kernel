#define export extern
#define external extern
#define internal static 
#define global_variable static 

#define kassert(expr) if(!(expr)) { stdout_write_fmt(&_iostate, "[ERROR] ASSERTION FIRED: " #expr); kpanic(); }

#define kerror(...) stdout_write_fmt(&_iostate, __VA_ARGS__)
#define klog(...) stdout_write_fmt(&_iostate, __VA_ARGS__)
#define kdebug(...) stdout_write_fmt(&_iostate, __VA_ARGS__)

#define klog_debug(...) stdout_write_fmt(&_iostate, __VA_ARGS__)
#define klog_error(...) stdout_write_fmt(&_iostate, __VA_ARGS__)

#define kpanic() kterm_redraw_if_required(&_kterm, &_iostate); \
	asm volatile ("cli"); \
	asm volatile ("hlt")

#define DEBUG_LOG(msg) bochs_write_string(msg)

#define COM1_PORT 0x3F8
#define COM2_PORT 0x2F8
#define COM3_PORT 0x3E8
#define COM4_PORT 0x2E8

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
