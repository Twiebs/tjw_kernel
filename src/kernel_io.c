static void
write_uint8_base16(char *dest, uint8_t value) {
  #define write_four_bits_base16(dest, four_bits) \
	if (four_bits > 9) { *dest = 'A' + (four_bits - 10); }  \
	else { *dest = '0' + four_bits; }
	uint8_t top_4_bits = (uint8_t)(value >> 4);
	uint8_t bottom_4_bits = (uint8_t)((uint8_t)(value << 4) >> 4);
	write_four_bits_base16(&dest[0], top_4_bits);
	write_four_bits_base16(&dest[1], bottom_4_bits);
  #undef write_four_bits_base16
}

static void
write_uint32_base16(char *dest, uint32_t value) {
	uint8_t byte3 = (uint8_t)(value >> 24);
	uint8_t byte2 = (uint8_t)(value >> 16);
	uint8_t byte1 = (uint8_t)(value >> 8);
	uint8_t byte0 = (uint8_t)value;
	write_uint8_base16(dest + 0, byte3);
	write_uint8_base16(dest + 2, byte2);
	write_uint8_base16(dest + 4, byte1);
	write_uint8_base16(dest + 6, byte0);
}

static void
append_console_entry(uint32_t entry, const void *data, uint32_t length, Console_Buffer *cb){
  size_t offset = cb->offset_of_entry[entry] + cb->length_of_entry[entry] % CONSOLE_OUTPUT_BUFFER_SIZE;
  if(offset + length > CONSOLE_OUTPUT_BUFFER_SIZE){
    size_t bytes_to_write = CONSOLE_OUTPUT_BUFFER_SIZE - (offset + length);
    memcpy(cb->output_buffer + offset, data, bytes_to_write);
    memcpy(cb->output_buffer, data, length - bytes_to_write);
  } else {
    memcpy(cb->output_buffer + offset, data, length);
  }
  cb->length_of_entry[entry] += length;
}

void 
console_write_fmt(Console_Buffer *cb, const char *fmt, ...) 
{
  if(globals.is_logging_disabled) return;

  va_list args;
  va_start(args,fmt);

  size_t previous_entry_index = (cb->entry_write_pos - 1) % CONSOLE_ENTRY_COUNT;
  size_t entry_index = cb->entry_write_pos % CONSOLE_ENTRY_COUNT;
  cb->offset_of_entry[entry_index] = cb->offset_of_entry[previous_entry_index] + cb->length_of_entry[previous_entry_index];
  if(cb->scroll_entry_index == ((entry_index - 1) % CONSOLE_ENTRY_COUNT)){
    cb->scroll_entry_index++;
  }
  cb->entry_write_pos++;
  if(cb->current_entry_count < CONSOLE_ENTRY_COUNT){
    cb->current_entry_count++;
  }

  size_t fmt_index = 0;
  while(fmt[fmt_index] != 0){
    if(fmt[fmt_index] == '%'){
			fmt_index++;
			if(fmt[fmt_index] == 'l') {
				fmt_index++;

        //uint64_t base16
				if(fmt[fmt_index] == 'u') 
        {
					uint64_t value = va_arg(args, uint64_t );
					uint32_t dword0 = (uint32_t)(value >> 32);
					uint32_t dword1 = (uint32_t)(value);
          char text[18];
          text[0] = '0';
          text[1] = 'x';
          write_uint32_base16(text + 2, dword0);
					write_uint32_base16(text + 10, dword1);
          append_console_entry(entry_index, text, 18, cb);
          fmt_index++;
				}
			}

      //uint32_t base16	
			else if (fmt[fmt_index] == 'u') 
      {
				uint32_t value = va_arg(args, uint32_t);
				uint8_t byte3 = (uint8_t)(value >> 24);
				uint8_t byte2 = (uint8_t)(value >> 16);
				uint8_t byte1 = (uint8_t)(value >> 8);
				uint8_t byte0 = (uint8_t)value;
        char text[10];
        text[0] = '0';
        text[1] = 'x';
				write_uint8_base16(text + 2, byte3);
				write_uint8_base16(text + 4, byte2);
				write_uint8_base16(text + 6, byte1);
				write_uint8_base16(text + 8, byte0);
        append_console_entry(entry_index, text, 10, cb);
        fmt_index++;
			}

      //cstring
			else if (fmt[fmt_index] == 's') 
      {
				const char *str = (const char *)va_arg(args, uintptr_t);
				size_t length = strlen(str);
        append_console_entry(entry_index, str, length, cb);
				fmt_index++;
			}

			else if (fmt[fmt_index] == '.') {
				fmt_index++;
				if(fmt[fmt_index] == '*'){
					fmt_index++;
					size_t str_length = (size_t)va_arg(args, uintptr_t);
					const char *str = (const char *)va_arg(args, uintptr_t);
          append_console_entry(entry_index, str, str_length, cb);
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
      append_console_entry(entry_index, &fmt[begin], length, cb);
		}
  } 

  size_t begin_offset = cb->offset_of_entry[entry_index];
  size_t entry_length = cb->length_of_entry[entry_index];
  if(begin_offset + entry_length > CONSOLE_OUTPUT_BUFFER_SIZE){
    size_t end_bytes_to_write = (begin_offset + entry_length) - CONSOLE_OUTPUT_BUFFER_SIZE;
    size_t first_bytes_to_write = entry_length - end_bytes_to_write;
    write_serial(cb->output_buffer + begin_offset, first_bytes_to_write);
    write_serial(cb->output_buffer, end_bytes_to_write);
    write_serial("\n", 1);
  } else {
    write_serial(cb->output_buffer + begin_offset, entry_length);
    write_serial("\n", 1);
  }

  cb->flags = cb->flags | Console_Flag_OUTPUT_DIRTY;
}

#if 0
internal void
stdout_write_fmt(IOState *io, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
  uint32_t index = 0;

	const char *buffer_begin = &io->output_buffer[io->output_buffer_count];
	char *write = &io->output_buffer[io->output_buffer_count];
	while (fmt[index] != 0) {
		if (fmt[index] == '%') {
			index++;

			if (fmt[index] == 'l') {
				index++;
				if (fmt[index] == 'u') {

					uint64_t value = va_arg(args, uint64_t );
					uint32_t dword0 = (uint32_t)(value >> 32);
					uint32_t dword1 = (uint32_t)(value);

					memcpy_literal_and_increment(write, "0x");
					write_uint32_base16(write + 0, dword0);
					write_uint32_base16(write + 8, dword1);
					write += 16;
					index++;
				}
			}
			
			else if (fmt[index] == 'u') {
				uint32_t value = va_arg(args, uint32_t);
				uint8_t byte3 = (uint8_t)(value >> 24);
				uint8_t byte2 = (uint8_t)(value >> 16);
				uint8_t byte1 = (uint8_t)(value >> 8);
				uint8_t byte0 = (uint8_t)value;
				memcpy_literal_and_increment(write, "0x");
				write_uint8_base16(write + 0, byte3);
				write_uint8_base16(write + 2, byte2);
				write_uint8_base16(write + 4, byte1);
				write_uint8_base16(write + 6, byte0);
				write += 8;
				index++;
			}

			else if (fmt[index] == 's') {
				//TODO(Torin) all of this io output manipulation is not correct
				//at all... we need to check if it overflows and then handle that by 
				//flushing the output buffer to a file
				const char *str = (const char *)va_arg(args, size_t);
				size_t length = strlen(str);
				memcpy(write, str, length);
				write += length;
				index++;
			}

			else if (fmt[index] == '.') {
				index++;
				if (fmt[index] == '*') {
					index++;
					size_t string_length = (size_t)va_arg(args, size_t);
					const char *str = (const char *)va_arg(args, size_t);
					memcpy(write, str, string_length);
					write += string_length;
					index++;
				}
			}
		}
		else {
			*write = fmt[index];
			write++;
			index++;
		}
	}

	*write = 0;
	write++;

	size_t bytes_written = write - buffer_begin;
	io->output_buffer_count += bytes_written;
	io->output_buffer_entry_count++;
	io->is_output_buffer_dirty = true;


	for (size_t i = 0; i < bytes_written - 1; i++) {
	}
  
}
#endif

#if 0
internal void
io_write_cstr(IOState *io, const char *cstr) {
	size_t length = strlen(cstr);	
	if (io->output_buffer_count + length + 1 > sizeof(io->output_buffer)) {
		memset(io->output_buffer, 0, sizeof(io->output_buffer));
		klog_error("EXCEDED OUTPUT BUFFER");		
		kpanic();
	} else {
		memcpy(&io->output_buffer[io->output_buffer_count], cstr, length);
		io->output_buffer_count += length;
		io->output_buffer[io->output_buffer_count] = 0;
	}
}
#endif


void klog_disable(){
  globals.is_logging_disabled = true;
}

void klog_enable(){
  globals.is_logging_disabled = false;
}