
void shell_command_register(Command_Line_Shell *shell, const char *name, uint64_t parameter_count, Shell_Command_Procedure procedure) {

}

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

void shell_execute_command(Command_Line_Shell *shell) {
  if (shell->input_buffer_count > 0) {
    shell->requires_redraw = true;
    size_t length = shell->input_buffer_count;
    shell->input_buffer_count = 0;
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
        if(keyboard->is_key_down[KEYBOARD_SCANCODE1_LSHIFT] ||
          keyboard->is_key_down[KEYBOARD_SCANCODE1_RSHIFT]){
          ascii_character = SCANCODE_TO_UPERCASE_ACII[scancode];
        } else { ascii_character = SCANCODE_TO_LOWERCASE_ACII[scancode]; }
        if(ascii_character == 0) return;
        shell_add_input_character(shell, ascii_character);
        shell->requires_redraw = true;
      }
    }
  }
}

void shell_draw_if_required(Command_Line_Shell *shell, Circular_Log *log) {
  if (shell->requires_redraw == false) return;
  vga_clear_screen();
  size_t entry_count = log->entries_back - log->entries_front;
  size_t entries_to_draw = min((uint32_t)(25 - 1), entry_count);
  for (size_t i = 0; i < entries_to_draw; i++) {
    size_t entry_index = ((log->entries_back - shell->line_offset) - (entries_to_draw - i)) % CONSOLE_ENTRY_COUNT;
    Log_Entry *entry = &log->entries[entry_index];

    int chars_to_write = entry->length;
    chars_to_write -= shell->character_number;
    chars_to_write = max(0, chars_to_write);
    chars_to_write = min(chars_to_write, 80);

    VGA_Color color = VGA_Color_LIGHT_GRAY;
    if (entry->level == Log_Level_ERROR)   { color = VGA_Color_RED;    }
    if (entry->level == Log_Level_WARNING) { color = VGA_Color_YELLOW; }
    if (entry->level == Log_Level_INFO)    { color = VGA_Color_CYAN;   }
    
    for (int j = 0; j < chars_to_write; j++) {
      vga_set_char(entry->message[j + shell->character_number], color, j, i);
    }
  }

  size_t input_buffer_to_write = min(80, shell->input_buffer_count);
  for (size_t i = 0; i < input_buffer_to_write; i++) {
    vga_set_char(shell->input_buffer[i], VGA_Color_RED, i, 25 - 1); 
  }

  shell->requires_redraw = false;
}

void shell_update(Command_Line_Shell *shell) {
  shell_process_keyboard_input(shell, &globals.keyboard);
  keyboard_state_reset(&globals.keyboard);
  shell_draw_if_required(shell, &globals.log);
}

void shell_initialize(Command_Line_Shell *shell) {
  shell_command_register(shell, "help", 0, shell_command_help);
  shell_command_register(shell, "ls", 0, shell_command_ls);
  shell_command_register(shell, "cd", 1, shell_command_cd);
}