
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