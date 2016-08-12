#define LITERAL_STRLEN(literal) (sizeof(literal) - 1)

#define pow(base, exponent) __builtin_pow(base, exponent)
#define strcmp(a, b) __builtin_strcmp(a, b)
#define strncmp(a, b, num) __builtin_strncmp(a, b, num)
#define ldexp(x, exp) __builtin_ldexp(x, exp)

#define malloc(size) kmem_alloc(size)
#define free(size) kmem_free(size)
#define realloc(p, newsize) kmem_realloc(p, newsize)

#define memcpy_literal_and_increment(write,literal) \
	memcpy(write, literal, LITERAL_STRLEN(literal)); \
  write += LITERAL_STRLEN(literal)

#define memcpy(dest,src,size) __memcpy((uint8_t*)dest, (uint8_t*)src, size)
#define memset(dest,value,size) __memset((uint8_t*)dest, value, size)

void __memset(uint8_t *dest, uint8_t value, size_t size) {
  for (size_t i = 0; i < size; i++) {
		dest[i] = value;
	}
}

void __memcpy(uint8_t *dest, uint8_t *src, size_t size) {
	for (size_t i = 0; i < size; i++) {
		dest[i] = src[i];
	}
}

static inline
size_t strlen(const char *str) {
	size_t result = 0;
	while (str[result] != 0)
		result ++;
	return result;
}

#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

#define LITERAL_STRLEN(literal) (sizeof(literal) - 1)
#define string_matches_literal(string, len, lit) strings_match(string,len, lit, LITERAL_STRLEN(lit))

static int 
string_equals_string(const char *stringA, size_t lengthA, const char *stringB, size_t lengthB) {
	if (lengthA != lengthB) return 0;
	for (size_t i = 0; i < lengthA; i++) {
		if (stringA[i] != stringB[i]) {
			return 0;
		}
	}
	return 1;
}

static int
string_matches_string(const char *str0, size_t length, const char *str1){
  for(size_t i = 0; i < length; i++) 
    if(str0[i] != str1[i]) return 0;
  return 1;
}


int is_char_alpha(char c) {
	if ((c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z')) {
		return 1;
	}

	return 0;
}


