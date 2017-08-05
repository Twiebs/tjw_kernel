

void utf16_to_ascii(uint8_t *ascii, uint16_t *utf16, size_t utf16_length) {
  size_t step_count = utf16_length / 2;
  for (size_t i = 0; i < step_count; i++){
    ascii[i] = utf16[i];
  } 
}

size_t cstring_length(const char *cstring) {
  size_t result = 0;
  while (cstring[result] != 0)
    result ++;
  return result;
}

#define string_matches_literal(string, len, lit) strings_match(string,len, lit, LITERAL_STRLEN(lit))

int string_equals_string(const char *stringA, size_t lengthA, const char *stringB, size_t lengthB) {
  if (lengthA != lengthB) return 0;
  for (size_t i = 0; i < lengthA; i++) {
    if (stringA[i] != stringB[i]) {
      return 0;
    }
  }
  return 1;
}

int string_matches_string(const char *str0, size_t length, const char *str1) {
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

void string_inplace_reverse(char *str, size_t length) {
  size_t midpoint = length / 2;
  for(size_t i = 0; i < midpoint; i++){
    char oldValue = str[i];
    str[i] = str[length - 1 - i];
    str[length - 1 - i] = oldValue;
  }
} 

size_t uint64_to_string(char *dest, uint64_t value, uint8_t base) {
  size_t bytes_written = 0;
  if(value == 0) {
    dest[0] = '0';
    bytes_written = 1;
  } else {
    while(value > 0){
      static const char LOOKUP[] = "0123456789ABCDEF";
      dest[bytes_written++] = LOOKUP[value % base];
      value /= base;
    }
    string_inplace_reverse(dest, bytes_written);
  }
  return bytes_written;
}

size_t vsnprintf(char *buffer, size_t capacity, const char *fmt, va_list args) {
  size_t bytes_written = 0;
  size_t fmt_index = 0;
  while(fmt[fmt_index] != 0){
    if(fmt[fmt_index] == '%'){
      fmt_index++;

      if(fmt[fmt_index] == 'l') {
        fmt_index++;
        if(fmt[fmt_index] == 'u') {
          uint64_t value = va_arg(args, uint64_t);
          if(bytes_written + 20 > capacity) return bytes_written;
          bytes_written += uint64_to_string(buffer + bytes_written, value, 10);
          fmt_index++;
        }
      }

      //uint32_t base16 
      else if (fmt[fmt_index] == 'u') {
        uint32_t value = va_arg(args, uint32_t);
        if(bytes_written + 10 > capacity) return bytes_written;
        bytes_written += uint64_to_string(buffer + bytes_written, (uint64_t)value, 10);
        fmt_index++;
      }

      //cstring
      else if (fmt[fmt_index] == 's') {
        const char *str = (const char *)va_arg(args, uintptr_t);
        size_t length = cstring_length(str);
        if(bytes_written + length > capacity) return bytes_written;
        memory_copy(buffer + bytes_written, str, length);
        bytes_written += length; 
        fmt_index++;
      }

      else if(fmt[fmt_index] == 'X'){
        uint64_t value = va_arg(args, uint64_t);
        if(bytes_written + 16 > capacity) return bytes_written;
        bytes_written += uint64_to_string(buffer + bytes_written, value, 16);
        fmt_index++;
      }

      else if (fmt[fmt_index] == '.') {
        fmt_index++;
        if(fmt[fmt_index] == '*'){
          fmt_index++;
          if(fmt[fmt_index] == 's'){
            fmt_index++;
            size_t str_length = (size_t)va_arg(args, uintptr_t);
            const char *str = (const char *)va_arg(args, uintptr_t);
            if(bytes_written + str_length > capacity) return bytes_written;
            memory_copy(buffer + bytes_written, str, str_length);
            bytes_written += str_length;
          }
        }
      }
    }

    else {
      size_t begin = fmt_index;
      while(fmt[fmt_index] != '%' && fmt[fmt_index] != 0){
       fmt_index++; 
      }

      size_t length = fmt_index - begin;
      if(bytes_written + length > capacity) return bytes_written;
      memory_copy(buffer + bytes_written, fmt + begin, length);
      bytes_written += length;
    }
  }
  return bytes_written;
}