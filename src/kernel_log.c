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
size_t uint64_to_string(char *dest, uint64_t value, uint8_t base){
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

static size_t kernel_vsnprintf(char *buffer, size_t capacity, const char *fmt, va_list args) {
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
				size_t length = strlen(str);
        if(bytes_written + length > capacity) return bytes_written;
        memcpy(buffer + bytes_written, str, length);
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
            memcpy(buffer + bytes_written, str, str_length);
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
      memcpy(buffer + bytes_written, fmt + begin, length);
      bytes_written += length;
		}
  } 
  return bytes_written;
}

Circular_Log_Entry *klog_get_next_available_entry(Circular_Log *log){
  spinlock_aquire(&log->spinlock);
  size_t entry_index = log->entry_write_position % CIRCULAR_LOG_ENTRY_COUNT; 
  Circular_Log_Entry *entry = &log->entries[entry_index];
  log->entry_write_position++;
  if(log->current_entry_count < CIRCULAR_LOG_ENTRY_COUNT) {
    log->current_entry_count++; 
  }
  spinlock_release(&log->spinlock);
  return entry;
}

void klog_write_string(Circular_Log *log, const char *string, size_t length){
  if(length > CIRCULAR_LOG_MESSAGE_SIZE){
    klog_error("failed to print string of length: %u", length);
    klog_error("string: %.*s", 30, string);
    return;
  }

  if(globals.is_logging_disabled) return;
  Circular_Log_Entry *entry = klog_get_next_available_entry(log);
  memcpy(entry->message, string, length);
  entry->length = length;

  spinlock_aquire(&log->spinlock);
  log->is_dirty = true;
  write_serial(entry->message, entry->length);
  write_serial("\n", 1);
  spinlock_release(&log->spinlock);
}

void klog_enable_category(Circular_Log *log, Log_Category category) {
  log->category_states[category] = Log_Category_State_ENABLED;
}

void klog_disable_category(Circular_Log *log, Log_Category category) {
  log->category_states[category] = Log_Category_State_DISABLED;
}

void klog_write_fmt(Circular_Log *log, Log_Category category, Log_Level level, const char *fmt, ...) {
  if(globals.is_logging_disabled) return;
  if(log->category_states[category] == Log_Category_State_DISABLED) return;

  asm volatile("cli");
  spinlock_aquire(&log->spinlock); 
  size_t entry_index = log->entry_write_position % CIRCULAR_LOG_ENTRY_COUNT;
  Circular_Log_Entry *entry = &log->entries[entry_index];
  entry->level = level;
  log->entry_write_position++;
  if(log->current_entry_count < CIRCULAR_LOG_ENTRY_COUNT){ log->current_entry_count++; }
  spinlock_release(&log->spinlock);
  asm volatile("sti");
  va_list args;
  va_start(args, fmt);
  entry->length = kernel_vsnprintf(entry->message, CIRCULAR_LOG_MESSAGE_SIZE, fmt, args);
  va_end(args);
  //NOTE(Torin: 2016-08-08) This could cause serial output to mismatch
  //console output if a cpu core formats a sufficantly small message and aquires
  //the spin lock before the first core finishes formating the message
  asm volatile("cli");
  spinlock_aquire(&log->spinlock);
  log->is_dirty = true;
  write_serial(entry->message, entry->length);
  write_serial("\n", 1);
#if FORCE_REDRAW_ON_LOG_ENTRY
  kgfx_draw_log_if_dirty(log);
#endif//FORCE_DREDRAW_ON_LOG_ENTRY
  spinlock_release(&log->spinlock);
  asm volatile("sti");
}

void klog_add_input_character(Circular_Log *log, const char c){
  if(c < ' ' || c > '~') return;
  spinlock_aquire(&log->spinlock);
  if(log->input_buffer_count > sizeof(log->input_buffer)) {
    spinlock_release(&log->spinlock);
    return;
  }
  log->input_buffer[log->input_buffer_count++] = c;
  log->is_dirty = true;
  spinlock_release(&log->spinlock);
}

void klog_remove_last_input_character(Circular_Log *log){
  spinlock_aquire(&log->spinlock);
  if(log->input_buffer_count > 0) {
    log->input_buffer[log->input_buffer_count] = 0;
    log->input_buffer_count -= 1;
  }
  log->is_dirty = true;
  spinlock_release(&log->spinlock);
}

void klog_submit_input_to_shell(Circular_Log *log){
  if(log->input_buffer_count > 0){
    log->is_dirty = true;
    size_t length = log->input_buffer_count;
    log->input_buffer_count = 0;
    kshell_process_command(log->input_buffer, length);
  } 
}

	

void klog_disable(){
  globals.is_logging_disabled = true;
}

void klog_enable(){
  globals.is_logging_disabled = false;
}