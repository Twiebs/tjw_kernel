


#define COMMAND_UNKNOWN_COMMAND 1
#define COMMAND_INVALID_ARG_COUNT 2

internal int 
is_command_signature_valid(const char *input, size_t input_len,
		const char *command_name, size_t command_length,
		uint32_t expected_arg_count, uint32_t actual_arg_count) 
{
	if (strings_match(input, input_len, command_name, command_length)) {
		if (expected_arg_count != actual_arg_count) {
			klog_error("%s expected %u arguments, only %u were provided", command_name, expected_arg_count, actual_arg_count);
			return COMMAND_INVALID_ARG_COUNT;
		} else {
			return 0;
		}
	}
	return COMMAND_UNKNOWN_COMMAND;
}

#if 0
internal bool 
process_shell_command(const char *command) {
		const char *procedure_name = command;
		size_t arg_count = 0;
		const char *args[6];
		size_t arg_lengths[6];

		const char *current = command;
		while (is_char_alpha(*current)) current++;
		size_t procedure_name_length = current - procedure_name;
		while (*current == ' ') {
			current++; 
			args[arg_count] = current;
			while (*current != ' ' && *current != 0) current++;
			arg_lengths[arg_count] = current - args[arg_count];
			arg_count++;
		}
		
#define command_hook(name, argc)  \
		else if (!(command_validity_status = is_command_signature_valid(procedure_name, procedure_name_length,  \
						name, LITERAL_STRLEN(name), argc, arg_count)))

		bool command_was_handled = false;
		int command_validity_status = 0;

		if (0) {}
		command_hook("reboot", 0) { kernel_reboot(); }

		command_hook("ls", 0) {
			for (uint32_t i = 0; i < _fs.kfs->node_count; i++) {
				KFS_Node *node = &_fs.kfs_nodes[i];
				klog_info("filename: %s, filesize: %u", node->name, node->size);
			}
		}

		command_hook("print", 1) {
			KFS_Node *node = kfs_find_file_with_name(args[0], arg_lengths[0]);
			if (node == 0) {
				klog_info("file %.*s could not be found", arg_lengths[0], args[0]);
			} else {
				const uint8_t *node_data = _fs.base_data + node->offset;
				klog_info("[%s]: %.*s", node->name, node->size, node_data);
			}
		}



		if (command_validity_status == COMMAND_UNKNOWN_COMMAND) {
			klog_info("unknown command '%s'", command);
			return false;
		} else if (command_validity_status == COMMAND_INVALID_ARG_COUNT) {
			return false;
		}


		return true;
}
#endif

void redraw_vga_text_terminal_if_dirty(VGA_Text_Terminal *kterm, Console_Buffer *cb)
{
	static const uint8_t VGA_TEXT_COLUMN_COUNT = 80;
	static const uint8_t VGA_TEXT_ROW_COUNT = 25;
	static uint8_t *VGA_TEXT_BUFFER = (uint8_t*)(0xB8000);
	static const uint32_t VGA_TEXT_INDEX_MAX = 2000;

  if((cb->flags & Console_Flag_OUTPUT_DIRTY) == 0) return;

  uint32_t top_entry_index = cb->scroll_entry_index;
  uint32_t entries_to_draw = 0;
  uint32_t current_line_count = 0;
  while(current_line_count < VGA_TEXT_ROW_COUNT){
    int64_t entry_length = cb->length_of_entry[top_entry_index];
    while(entry_length > 0){
      current_line_count++;
      entry_length -= VGA_TEXT_COLUMN_COUNT; 
      if(current_line_count > VGA_TEXT_ROW_COUNT) break;
    }
    entries_to_draw++;
    if(entries_to_draw >= cb->current_entry_count){
      break;
    }
    top_entry_index = (top_entry_index - 1)  % CONSOLE_ENTRY_COUNT;
  }

  uint8_t current_color = VGAColor_GREEN;
  uint8_t current_row = 0, current_column = 0;
  memset(VGA_TEXT_BUFFER, 0, (VGA_TEXT_COLUMN_COUNT * VGA_TEXT_ROW_COUNT) * 2);

  #define write_vga_data(buffer, buffer_length) \
    for(size_t buffer_offset= 0; buffer_offset < buffer_length; buffer_offset++){ \
      size_t vga_index = ((current_row * VGA_TEXT_COLUMN_COUNT) + current_column) * 2; \
      VGA_TEXT_BUFFER[vga_index] = buffer[buffer_offset]; \
      VGA_TEXT_BUFFER[vga_index+1] = current_color; \
      current_column++; \
      if(current_column > VGA_TEXT_COLUMN_COUNT) { \
        current_column = 0; \
        current_row++; \
      } \
    }
      
  for(size_t i = 0; i < entries_to_draw; i++){
    size_t current_entry_index = (top_entry_index + i) % CONSOLE_ENTRY_COUNT;
    size_t entry_start_offset = cb->offset_of_entry[current_entry_index];
    size_t entry_length = cb->length_of_entry[current_entry_index];
    size_t entry_end_offset = entry_start_offset + entry_length;
    if(entry_start_offset + entry_length > CONSOLE_OUTPUT_BUFFER_SIZE){
      size_t end_bytes_to_write = entry_end_offset - CONSOLE_OUTPUT_BUFFER_SIZE;
      size_t start_bytes_to_write = entry_length - end_bytes_to_write;
      write_vga_data((cb->output_buffer + entry_start_offset), start_bytes_to_write);
      write_vga_data((cb->output_buffer), end_bytes_to_write);
    } else {
      write_vga_data((cb->output_buffer + entry_start_offset), entry_length);
    }
    current_row++;
    current_column = 0;
  }

  cb->flags = cb->flags & ~Console_Flag_OUTPUT_DIRTY;
}


#if 0	
	if (io->is_command_ready) {
		process_shell_command(io->input_buffer);
		io->is_command_ready = false;
		io->input_buffer_count = 0;
		memset(io->input_buffer, 0, sizeof(io->input_buffer));
		io->is_input_buffer_dirty = true;
	}
#endif

#if 0
  if (io->is_output_buffer_dirty) {
				
		const char *read = kterm->top_entry;
		while (*read != 0 && current_row < 25) {
			size_t index = ((current_row * VGA_TEXT_COLUMN_COUNT) + current_column) * 2;
			VGA_TEXT_BUFFER[index] = *read;
			VGA_TEXT_BUFFER[index+1] = current_color;
			current_column++;
			if (current_column > VGA_TEXT_COLUMN_COUNT) {
				current_column = 2;
				current_row++;
			}
			read++;
			if (*read == 0) {
				read++;
				current_row++;
				current_column = 0;
			} 
		}
		io->is_output_buffer_dirty = false;
	}

	if (io->is_input_buffer_dirty) {
		uint32_t current_row = VGA_TEXT_ROW_COUNT - 1;
		uint32_t current_column = 0;
		uint32_t index = ((current_row * VGA_TEXT_COLUMN_COUNT) + current_column) * 2;

		VGA_TEXT_BUFFER[index] = '>';
		VGA_TEXT_BUFFER[index+1] = VGAColor_GREEN;
		index += 2;
		current_column += 1;
	
		for (uint32_t i = 0; i < io->input_buffer_count; i++) {
		  VGA_TEXT_BUFFER[index] = io->input_buffer[i];
			VGA_TEXT_BUFFER[index+1] = VGAColor_GREEN;
			index += 2;
			current_column++;
		}

		uint32_t diff = VGA_TEXT_COLUMN_COUNT - current_column;
		for (uint32_t i = 0; i < diff; i+=2) {
			VGA_TEXT_BUFFER[index] = 0;
			VGA_TEXT_BUFFER[index+1] = 0;
			index += 2;
		}

		io->is_input_buffer_dirty = false;
	}
}

  #endif

