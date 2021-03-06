#define LITERAL_STRLEN(literal) (sizeof(literal) - 1)

typedef struct {
  uint8_t *string;
  size_t length;
} String_Span;

typedef struct {
  char data[128];
  int64_t length;
} String_Buffer_128;

int64_t string_buffer_128_set_cstring(String_Buffer_128 *string_buffer, const char *cstring);
int64_t string_buffer_128_append_cstring(String_Buffer_128 *string_buffer,const char *cstring);
int64_t string_buffer_128_set_string(String_Buffer_128 *string_buffer,const char *string, int64_t length);
int64_t string_buffer_128_append_string(String_Buffer_128 *string_buffer,const char *string, int64_t length);
int64_t string_buffer_128_set_span(String_Buffer_128 *string_buffer,String_Span *span);
int64_t string_buffer_128_append_span(String_Buffer_128 *string_buffer,String_Span *span);

#define literal_string_span(string) (String_Span { string, sizeof(string) - 1 })

void utf16_to_ascii(uint8_t *ascii, uint16_t *utf16, size_t utf16_length);
size_t uint64_to_string(char *dest, uint64_t value, uint8_t base);

int string_equals_string(const char *stringA, size_t lengthA, const char *stringB, size_t lengthB);
int string_matches_string(const char *str0, size_t length, const char *str1);
void string_inplace_reverse(char *str, size_t length);

//@returns index of first non-whitespace char after start_offset
size_t string_seek_past_whitespaces(const char *string_begin, size_t start_offset, size_t max_seek_distance);
//@returns index of first whitespace char after start_offset
size_t string_seek_next_whitespace(const char *string_begin, size_t start_offset, size_t max_seek_distance);
//@returns length of the substring
size_t string_substring_to_next_whitespace(const char *string, size_t offset, size_t length);

//C-Strings (Null terminated)
size_t cstring_length(const char *cstring);
size_t cstring_copy_to_buffer_unsafe(const char *cstring, char *buffer);
size_t cstring_substring_to_next_whitespace(const char *cstring);

bool char_is_alpha(char c);
bool char_is_whitespace(char c);


size_t vsnprintf(char *buffer, size_t capacity, const char *fmt, va_list args);