
internal void
io_write_cstr(IOState *io, const char *cstr) {
	size_t length = strlen(cstr);	
	if (io->output_buffer_count + length + 1 > sizeof(io->output_buffer)) {
		memset(io->output_buffer, 0, sizeof(io->output_buffer));
		kpanic("EXCEDED OUTPUT BUFFER");		
		asm volatile ("cli");
		asm volatile ("hlt");
	} else {
		memcpy(&io->output_buffer[io->output_buffer_count], cstr, length);
		io->output_buffer_count += length;
		io->output_buffer[io->output_buffer_count] = 0;
	}
}

internal void
write_byte_as_hex(char *dest, uint8_t value) {
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
stdout_write_fmt(IOState *io, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
  uint32_t index = 0;

	const char *buffer_begin = &io->output_buffer[io->output_buffer_count];
	char *write = &io->output_buffer[io->output_buffer_count];
	while (fmt[index] != 0) {
		if (fmt[index] == '%') {
			index++;
			
			if (fmt[index] == 'u') {
				uint32_t value = va_arg(args, uint32_t);

				uint8_t byte3 = (uint8_t)(value >> 24);
				uint8_t byte2 = (uint8_t)(value >> 16);
				uint8_t byte1 = (uint8_t)(value >> 8);
				uint8_t byte0 = (uint8_t)value;
					
				memcpy_literal_and_increment(write, "0x");
				write_byte_as_hex(write + 0, byte3);
				write_byte_as_hex(write + 2, byte2);
				write_byte_as_hex(write + 4, byte1);
				write_byte_as_hex(write + 6, byte0);
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

#ifdef KERNEL_BUILD_DEBUG
	for (size_t i = 0; i < bytes_written; i++) {
		write_port(COM1_PORT, buffer_begin[i]);
	}
	write_port(COM1_PORT, '\n');
#endif
}


