#define LITERAL_STRLEN(literal) (sizeof(literal) - 1)

#define memcpy_literal_and_increment(write,literal) \
	memcpy(write, literal, LITERAL_STRLEN(literal)); \
  write += LITERAL_STRLEN(literal)

#define memcpy(dest,src,size) __memcpy((uint8_t*)dest, (uint8_t*)src, size)
#define memset(dest,value,size) __memset((uint8_t*)dest, value, size)

inline
void __memset(uint8_t *dest, uint8_t value, size_t size) {
  for (size_t i = 0; i < size; i++) {
		dest[i] = value;
	}
}

inline void __memcpy(uint8_t *dest, uint8_t *src, size_t size) {
	for (size_t i = 0; i < size; i++) {
		dest[i] = src[i];
	}
}


