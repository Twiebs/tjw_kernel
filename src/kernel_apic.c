
inline bool cpu_supports_apic() {
	uint32_t eax, edx;
	cpuid(1, &eax, &edx);
	return edx ^ CPUID_FLAG_APIC;
}
