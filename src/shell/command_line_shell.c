


void shell_add_input_character(Command_Line_Shell *shell, const char c) {
  if (c < ' ' || c > '~') return;
  if (shell->input_buffer_count > sizeof(shell->input_buffer)) {
    return;
  }
  shell->input_buffer[shell->input_buffer_count++] = c;
  shell->requires_redraw = true;
}

void shell_remove_last_input_character(Command_Line_Shell *shell) {
  if (shell->input_buffer_count > 0) {
    shell->input_buffer_count -= 1;
    shell->input_buffer[shell->input_buffer_count] = 0;
    shell->requires_redraw = true;
  }
}

void shell_clear_input_buffer(Command_Line_Shell *shell) {
  shell->requires_redraw = true;
  memory_set(shell->input_buffer, 0x00, shell->input_buffer_count);
  shell->input_buffer_count = 0;
}


void shell_execute_command(Command_Line_Shell *shell) {
  if (shell->input_buffer_count > 0) {
    klog_info("> %s", shell->input_buffer);

    Shell_Command_Parameter_Info parameter_info = {};
    size_t current_index = 0;
    size_t command_string_length = cstring_substring_to_next_whitespace(shell->input_buffer);
    current_index = command_string_length;

    while (current_index < shell->input_buffer_count) {
      current_index = string_seek_past_whitespaces(shell->input_buffer, current_index, shell->input_buffer_count);
      if (current_index != shell->input_buffer_count) {
        if (parameter_info.parameter_count >= ARRAY_COUNT(parameter_info.parameters)) break;
        Shell_Command_Parameter *param = &parameter_info.parameters[parameter_info.parameter_count++];
        param->text = &shell->input_buffer[current_index];
        param->length = string_substring_to_next_whitespace(shell->input_buffer, current_index, shell->input_buffer_count);
        current_index += param->length;
      }
    }

    if (command_string_length == 0) {
      klog_error("command cannot start with whitespace");
    } else {
      for (size_t i = 0; i < shell->command_count; i++) {
        Shell_Command *command = &shell->commands[i];
        if (string_equals_string(shell->input_buffer, command_string_length, command->name, command->name_count)) {
          if (command->parameter_count != parameter_info.parameter_count) {
            klog_error("invalid command parameter count for command %.*s", command->name_count, command->name);
            shell_clear_input_buffer(shell);
            return;
          } else { 
            command->procedure(&parameter_info);
            klog_debug("returned!");
            shell_clear_input_buffer(shell);
            return;
          }
        }
      }

      klog_error("cannot find command %.*s", (int)command_string_length, shell->input_buffer);
    }


    shell_clear_input_buffer(shell);
  }
}

void shell_process_keyboard_input(Command_Line_Shell *shell, Keyboard_State *keyboard) {
  Circular_Log *log = &globals.log;

  for (size_t i = 0; i < sizeof(keyboard->is_key_pressed); i++) {
    if (keyboard->is_key_pressed[i]) {
      uint8_t scancode = i;

      if (scancode == KEYBOARD_SCANCODE1_BACKSPACE_PRESSED) {
        shell_remove_last_input_character(shell);
      } else if (scancode == KEYBOARD_SCANCODE1_ENTER_PRESSED) {
        shell_execute_command(shell);
      } else if (scancode == KEYBOARD_SCANCODE1_RIGHT_PRESSED) {
        shell->character_number++;
        shell->requires_redraw = true;
      } else if (scancode == KEYBOARD_SCANCODE1_LEFT_PRESSED) {
        if (shell->character_number > 0) shell->character_number--;
        shell->requires_redraw = true;
      } else if (scancode == KEYBOARD_SCANCODE1_UP_PRESSED) {
        if (shell->line_offset < ARRAY_COUNT(log->entries)) {
          shell->line_offset += 1;
          shell->requires_redraw = true;
        }

        shell->requires_redraw = true;
      } else if (scancode == KEYBOARD_SCANCODE1_DOWN_PRESSED){
        if (shell->line_offset > 0) {
          shell->line_offset -= 1;
          shell->requires_redraw = true;
        }
      }

      if (scancode < (int)sizeof(SCANCODE_TO_LOWERCASE_ACII)) {
        char ascii_character = 0;
        if (keyboard->is_key_down[Keyboard_Keycode_LEFT_SHIFT]) {
          ascii_character = SCANCODE_TO_UPERCASE_ACII[scancode];
        } else { ascii_character = SCANCODE_TO_LOWERCASE_ACII[scancode]; }
        if(ascii_character == 0) return;
        shell_add_input_character(shell, ascii_character);
        shell->requires_redraw = true;
      }
    }
  }
}

void shell_draw_if_required(Command_Line_Shell *shell, Circular_Log *log)
{
    if (shell->requires_redraw == false)
    {
        return;
    }

    vga_clear_screen();

    kassert(log->entries_back >= log->entries_front);
    const uint64_t entry_count = log->entries_back - log->entries_front;
    kassert(entry_count <= CIRCULAR_LOG_ENTRY_COUNT);

    // VGA_TEXT_ROW_COUNT - 1 Because we need one line to draw user input.
    static const uint64_t AVAILABLE_LINE_COUNT = VGA_TEXT_ROW_COUNT - 1;
    const uint64_t entries_to_draw = min_uint64(AVAILABLE_LINE_COUNT, entry_count);
    kassert(entries_to_draw <= AVAILABLE_LINE_COUNT);

    for (size_t i = 0; i < entries_to_draw; i++) 
    {
        size_t entry_index = ((log->entries_back - shell->line_offset) - (entries_to_draw - i)) % CONSOLE_ENTRY_COUNT;

        const Log_Entry *entry = &log->entries[entry_index];


        VGA_Color color = VGA_Color_LIGHT_GRAY;
        if (entry->log_level == Log_Level_ERROR)   { color = VGA_Color_RED;    }
        if (entry->log_level == Log_Level_WARNING) { color = VGA_Color_YELLOW; }
        if (entry->log_level == Log_Level_INFO)    { color = VGA_Color_CYAN;   }

        const char *entry_tag_name = LOG_CATEGORY_TAGS[entry->log_category];
        if (entry->log_category == Log_Category_DEFAULT) entry_tag_name = 0;
        size_t entry_tag_name_length = cstring_length(entry_tag_name);


        size_t length_of_tag_to_write = 0;
        if (shell->character_number < entry_tag_name_length) {
          length_of_tag_to_write = entry_tag_name_length - shell->character_number;
          vga_write_string(entry_tag_name + shell->character_number, length_of_tag_to_write, color, 0, i);
        }

        size_t message_offset = 0;
        if (shell->character_number > entry_tag_name_length) {
          message_offset = shell->character_number - entry_tag_name_length;
        }

        int message_chars_to_write = entry->message_length;
        message_chars_to_write -= message_offset;
        message_chars_to_write = max(0, message_chars_to_write);
        message_chars_to_write = min((uint64_t)message_chars_to_write, shell->characters_per_line);
        vga_write_string(entry->message + message_offset, message_chars_to_write, color, length_of_tag_to_write, i);
    }

    size_t input_buffer_to_write = min(shell->characters_per_line, shell->input_buffer_count);
    for (size_t i = 0; i < input_buffer_to_write; i++) {
      vga_set_char(shell->input_buffer[i], VGA_Color_RED, i, 25 - 1); 
    }

    shell->requires_redraw = false;
}

void shell_update(Command_Line_Shell *shell) {
  keyboard_state_update(&globals.keyboard);
  shell_process_keyboard_input(shell, &globals.keyboard);
  keyboard_state_reset(&globals.keyboard);
  shell_draw_if_required(shell, &globals.log);
}


void shell_command_register(Command_Line_Shell *shell, const char *name, uint64_t parameter_count, Shell_Command_Procedure procedure) {
  if (shell->command_count >= ARRAY_COUNT(shell->commands)) return;
  size_t name_length = cstring_length(name);
  if (name_length > 128) { //TODO shell name length
    klog_error("cannot register command %s. Its too big", name);
    return;
  }

  Shell_Command *command = &shell->commands[shell->command_count++];
  memory_copy(command->name, name, name_length);
  command->name_count = name_length;
  command->procedure = procedure;
  command->parameter_count = parameter_count;
}

void shell_initialize(Command_Line_Shell *shell) {
  shell->current_directory[0] = '/';
  shell->current_directory_count = 1;
  shell->characters_per_line= 80;
  shell_command_register(shell, "help", 0, shell_command_help);
  shell_command_register(shell, "ls", 0, shell_command_ls);
  shell_command_register(shell, "cd", 1, shell_command_cd);
  shell_command_register(shell, "cat", 1, shell_command_cat);
  shell_command_register(shell, "lspci", 0, shell_command_lspci);
  shell_command_register(shell, "run", 1, shell_command_run);
  shell_command_register(shell, "q", 0, shell_command_lazy);
}