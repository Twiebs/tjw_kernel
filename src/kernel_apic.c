
static inline void ioapic_write_register(const uintptr_t ioapic_base, const uint8_t offset, const uint32_t value);
static inline uint32_t ioapic_read_register(const uintptr_t ioapic_base, const uint8_t offset);
static inline void lapic_write_register(uintptr_t lapic_base, uintptr_t register_offset, uint32_t value);

static inline
void ioapic_write_register(const uintptr_t ioapic_base, const uint8_t offset, const uint32_t value){
  uint32_t volatile *ioapic = (uint32_t volatile *)ioapic_base;
  ioapic[0] = offset;
  ioapic[4] = value;
}

static inline
uint32_t ioapic_read_register(const uintptr_t ioapic_base, const uint8_t offset){
  uint32_t volatile *ioapic = (uint32_t volatile*)ioapic_base;
  ioapic[0] = offset;
  return ioapic[4];
}

static inline
void lapic_write_register(uintptr_t lapic_base, uintptr_t register_offset, uint32_t value){
  uint32_t volatile *lapic = (uint32_t volatile *)(lapic_base + register_offset);
  lapic[0] = value;
}