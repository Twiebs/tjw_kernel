
#define memcpy_literal_and_increment(write,literal) \
  memcpy(write, literal, LITERAL_STRLEN(literal)); \
  write += LITERAL_STRLEN(literal)

void memory_copy(void *dest, const void *src, size_t size) {
  uint8_t *d = (uint8_t *)dest;
  uint8_t *s = (uint8_t *)src;
  for (size_t i = 0; i < size; i++) {
    d[i] = s[i];
  }
}

void memory_set(void *dest, uint8_t value, size_t size) {
  uint8_t *d = (uint8_t *)dest;
  for (size_t i = 0; i < size; i++) {
    d[i] = value;
  }
}

uint64_t memory_align(uint64_t address, uint64_t align) {
  uint64_t result = (address + (align - 1)) & (~(align - 1));
  return result;
}

void unpack_32_2x16(uint32_t value, uint16_t *a, uint16_t *b) {
  *a = (value >> 16) & 0xFFFF;
  *b = (value & 0xFFFF);
}

void unpack_32_4x8(uint32_t value, uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d) {
  *a = (value >> 24) & 0xFF;
  *b = (value >> 16) & 0xFF;
  *c = (value >>  8) & 0xFF;
  *d = (value >>  0) & 0xFF;
}

void log_bytes(void *data, size_t size) {
  uint8_t *bytes = (uint8_t *)data;
  size_t row_count = size / 8;
  for (size_t i = 0; i < row_count; i++) {
    klog_debug("  0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X",
      bytes[i*8 + 0], bytes[i*8 + 1], bytes[i*8 + 2], bytes[i*8 + 3], 
      bytes[i*8 + 4], bytes[i*8 + 5], bytes[i*8 + 6], bytes[i*8 + 7]);
  }

  size_t remainder = size - (row_count * 8);

}

uint32_t mmio_register_read32(uint64_t base, uint64_t offset) {
  volatile uint32_t *address = (volatile uint32_t *)(base + offset);
  uint32_t result = *address;
  return result;
}

void mmio_register_write32(uint64_t base, uint64_t offset, uint32_t value) {
  volatile uint32_t *address = (volatile uint32_t *)(base + offset);
  *address = value;
}