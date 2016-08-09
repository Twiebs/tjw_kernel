
#if 0
static inline 
void uint8_to_string_base16(char *dest, uint8_t value){
  #define write_four_bits_base16(dest, four_bits) \
	if (four_bits > 9) { *dest = 'A' + (four_bits - 10); }  \
	else { *dest = '0' + four_bits; }
	uint8_t top_4_bits = (uint8_t)(value >> 4);
	uint8_t bottom_4_bits = (uint8_t)((uint8_t)(value << 4) >> 4);
	write_four_bits_base16(&dest[0], top_4_bits);
	write_four_bits_base16(&dest[1], bottom_4_bits);
  #undef write_four_bits_base16
}

static inline
void uint32_to_string_base16(char *dest, uint32_t value){
	uint8_t byte3 = (uint8_t)(value >> 24);
	uint8_t byte2 = (uint8_t)(value >> 16);
	uint8_t byte1 = (uint8_t)(value >> 8);
	uint8_t byte0 = (uint8_t)value;
	uint8_to_string_base16(dest + 0, byte3);
	uint8_to_string_base16(dest + 2, byte2);
	uint8_to_string_base16(dest + 4, byte1);
	uint8_to_string_base16(dest + 6, byte0);
}

static inline
void uint64_to_string_base16(char *dest, uint64_t value){
  uint32_t value0 = (uint32_t)(value >> 32);
  uint32_t value1 = (uint32_t)(value);
  uint32_to_string_base16(dest + 0, value1);
  uint32_to_string_base16(dest + 8, value0);
}
#endif

static inline
void string_inplace_reverse(char *str, size_t length){
  size_t midpoint = length / 2;
  for(size_t i = 0; i < midpoint; i++){
    char oldValue = str[i];
    str[i] = str[length - 1 - i];
    str[length - 1 - i] = oldValue;
  }
} 

static inline 
size_t uint64_to_string_base16(char *dest, uint64_t value){
  size_t bytes_written = 0;
  if(value == 0) {
    dest[0] = '0';
    bytes_written = 1;
  } else {
    while(value > 0){
      static const char LOOKUP[] = "0123456789ABCDEF";
      dest[bytes_written++] = LOOKUP[value % 16];
      value /= 16;
    }
    string_inplace_reverse(dest, bytes_written);
  }
  return bytes_written;
}

static size_t 
kernel_vsnprintf(char *buffer, size_t capacity, const char *fmt, va_list args) 
{
  size_t bytes_written = 0;
  size_t fmt_index = 0;
  while(fmt[fmt_index] != 0){
    if(fmt[fmt_index] == '%'){
			fmt_index++;

			if(fmt[fmt_index] == 'l') {
				fmt_index++;
				if(fmt[fmt_index] == 'u') {
					uint64_t value = va_arg(args, uint64_t);
          if(bytes_written + 18 > capacity) return bytes_written;
          buffer[bytes_written + 0] = '0';
          buffer[bytes_written + 1] = 'x';
          bytes_written += 2;
          bytes_written += uint64_to_string_base16(buffer + bytes_written, value);
          fmt_index++;
				}
			}

      //uint32_t base16	
			else if (fmt[fmt_index] == 'u') {
				uint32_t value = va_arg(args, uint32_t);
        if(bytes_written + 10 > capacity) return bytes_written;
        buffer[bytes_written + 0] = '0';
        buffer[bytes_written + 1] = 'x';
        bytes_written += 2;
				bytes_written += uint64_to_string_base16(buffer + bytes_written, (uint64_t)value);
        fmt_index++;
			}

      //cstring
			else if (fmt[fmt_index] == 's') {
				const char *str = (const char *)va_arg(args, uintptr_t);
				size_t length = strlen(str);
        if(bytes_written + length > capacity) return bytes_written;
        memcpy(buffer + bytes_written, str, length);
        bytes_written += length; 
				fmt_index++;
			}

			else if (fmt[fmt_index] == '.') {
				fmt_index++;
				if(fmt[fmt_index] == '*'){
					fmt_index++;
					size_t str_length = (size_t)va_arg(args, uintptr_t);
					const char *str = (const char *)va_arg(args, uintptr_t);
          if(bytes_written + str_length > capacity) return bytes_written;
          memcpy(buffer + bytes_written, str, str_length);
          bytes_written += str_length;
					fmt_index++;
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
      memcpy(buffer + bytes_written, fmt + begin, length);
      bytes_written += length;
		}
  } 
  return bytes_written;
}

void klog_write_fmt(Circular_Log *log, const char *fmt, ...){
  if(globals.is_logging_disabled) return;

  spinlock_aquire(&log->spinlock); 
  size_t entry_index = log->entry_write_position % CIRCULAR_LOG_ENTRY_COUNT;
  Circular_Log_Entry *entry = &log->entries[entry_index];
  log->entry_write_position++;
  log->current_scroll_position++;
  if(log->current_entry_count < CIRCULAR_LOG_ENTRY_COUNT){ log->current_entry_count++; }
  spinlock_release(&log->spinlock);
  
  va_list args;
  va_start(args, fmt);
  entry->length = kernel_vsnprintf(entry->message, CIRCULAR_LOG_MESSAGE_SIZE, fmt, args);
  va_end(args);

  //NOTE(Torin: 2016-08-08) This could cause serial output to mismatch
  //console output if a cpu core formats a sufficantly small message and aquires
  //the spin lock before the first core finishes formating the message
  spinlock_aquire(&log->spinlock);
  write_serial(entry->message, entry->length);
  write_serial("\n", 1);
  spinlock_release(&log->spinlock);
}

void klog_disable(){
  globals.is_logging_disabled = true;
}

void klog_enable(){
  globals.is_logging_disabled = false;
}