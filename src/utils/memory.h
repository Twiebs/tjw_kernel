#define ARRAY_COUNT(array) (sizeof(array)/sizeof(*array))

void memory_copy(void *dest, const void *src, size_t size);
void memory_set(void *dest, uint8_t value, size_t size);
uint64_t memory_align(uint64_t address, uint64_t align);

void unpack_32_4x8(uint32_t value, uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d);
void unpack_32_2x16(uint32_t value, uint16_t *a, uint16_t *b);

void log_bytes(void *data, size_t size);


uint32_t mmio_register_read32(uint64_t base, uint64_t offset);
void mmio_register_write32(uint64_t base, uint64_t offset, uint32_t value);