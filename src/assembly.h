
static inline void write_port_uint8(uint16_t port, uint8_t value) 
{
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void write_port_uint16(uint16_t port, uint16_t value) 
{
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void write_port_uint32(volatile uint16_t port, volatile uint32_t value)
{
    asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t read_port_uint8(uint16_t port) 
{
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline uint16_t read_port_uint16(uint16_t port)
{
    uint16_t result;
    asm volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline uint32_t read_port_uint32(uint16_t port)
{
    uint32_t result;
    asm volatile ("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

inline void cpuid(int code, uint32_t *a, uint32_t *d) 
{
    asm volatile("cpuid":"=a"(*a),"=d"(*d):"a"(code):"ecx","ebx");
}

inline void cpu_get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi) 
{
     asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}