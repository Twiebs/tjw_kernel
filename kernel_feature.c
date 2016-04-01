
external uint32_t cpuid_is_supported();
external uint32_t cpuid_is_longmode_supported();

internal inline
void cpuid(uint32_t code, uint32_t *a, uint32_t *d) {
	asm volatile ("cpuid":"=a"(*a), "=d"(*d):"a"(code): "ecx", "ebx");
}

namespace CPUID {
	static const uint32_t APIC_BIT = 1 << 9;
}

internal inline
bool cpuid_is_apic_supported() {
	uint32_t eax, edx;
	cpuid(1, &eax, &edx);
	return edx & CPUID::APIC_BIT;
}
