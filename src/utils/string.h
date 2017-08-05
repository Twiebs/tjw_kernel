#define LITERAL_STRLEN(literal) (sizeof(literal) - 1)

void utf16_to_ascii(uint8_t *ascii, uint16_t *utf16, size_t utf16_length);
size_t uint64_to_string(char *dest, uint64_t value, uint8_t base);

int string_equals_string(const char *stringA, size_t lengthA, const char *stringB, size_t lengthB);
int string_matches_string(const char *str0, size_t length, const char *str1);
void string_inplace_reverse(char *str, size_t length);

size_t cstring_length(const char *cstring);

size_t vsnprintf(char *buffer, size_t capacity, const char *fmt, va_list args);

int is_char_alpha(char c);