
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

internal void
write_uint8_as_hex(char *dest, uint8_t value) {
#define write_four_bits_as_hex(dest, four_bits) \
	if (four_bits > 9) { *dest = 'A' + (four_bits - 10); }  \
	else { *dest = '0' + four_bits; }
	uint8_t top_4_bits = (uint8_t)(value >> 4);
	uint8_t bottom_4_bits = (uint8_t)((uint8_t)(value << 4) >> 4);
	write_four_bits_as_hex(&dest[0], top_4_bits);
	write_four_bits_as_hex(&dest[1], bottom_4_bits);
#undef write_four_bits_as_hex
}

internal void
write_uint32_as_hex(char *dest, uint32_t value) {
	uint8_t byte3 = (uint8_t)(value >> 24);
	uint8_t byte2 = (uint8_t)(value >> 16);
	uint8_t byte1 = (uint8_t)(value >> 8);
	uint8_t byte0 = (uint8_t)value;
	write_uint8_as_hex(dest + 0, byte3);
	write_uint8_as_hex(dest + 2, byte2);
	write_uint8_as_hex(dest + 4, byte1);
	write_uint8_as_hex(dest + 6, byte0);
}

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
					write_uint32_as_hex(write + 0, dword0);
					write_uint32_as_hex(write + 8, dword1);
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
				write_uint8_as_hex(write + 0, byte3);
				write_uint8_as_hex(write + 2, byte2);
				write_uint8_as_hex(write + 4, byte1);
				write_uint8_as_hex(write + 6, byte0);
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
		write_serial(buffer_begin[i]);
	}
	write_serial('\n');
}

