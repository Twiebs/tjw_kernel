
static void desktop_enviroment_find_programs_callback(VFS_Node_Info *node_info, void *userdata) {
  Desktop_Enviroment *desktop_enviroment = (Desktop_Enviroment *)userdata;
  if (node_info->type != VFS_Node_Type_FILE) {
     return;
  }

  Search_Entry *entry = search_entry_array_add(&desktop_enviroment->search_entry_array);
  if (entry == NULL) {
    log_error(DESKTOP, "Could not add an additional search entry!");
    return;
  }

  entry->type = Search_Entry_Type_PROGRAM;
  if (node_info->name_length > sizeof(entry->search_string)) {
    log_error(DESKTOP, "Program name is too large to add to search entry");
    return;
  }

  memory_copy(entry->search_string, node_info->name, node_info->name_length);
  entry->search_string_length = node_info->name_length;
  log_debug(DESKTOP, "Added search entry %s", entry->search_string);
}

void desktop_enviroment_update_search_list(Desktop_Enviroment *desktop_enviroment) {
  VFS_Node_Handle program_directory_handle = {};
  if (vfs_acquire_node_handle("/programs", sizeof("/programs") - 1, &program_directory_handle)) {
    log_error(DESKTOP, "Could not open program directory to update program list");
    return;
  }

  vfs_node_iterate_directory(&program_directory_handle, desktop_enviroment_find_programs_callback, desktop_enviroment);
}

void desktop_enviroment_initialize(Desktop_Enviroment *desktop_enviroment, Graphics_Device *graphics_device) {
  desktop_enviroment->graphics_device = graphics_device;
  software_graphics_buffer_create(&desktop_enviroment->overlay_buffer, graphics_device->width, graphics_device->height);
  desktop_enviroment_update_search_list(desktop_enviroment);
  klog_debug("initialized desktop enviroment");
}

void desktop_enviroment_draw_desktop(Desktop_Enviroment *desktop_enviroment) {
  Color_RGB_U8 desktop_background_color = { 44, 62, 80 };
  Software_Graphics_Buffer current_back_buffer = {};
  graphics_device_get_back_buffer(desktop_enviroment->graphics_device, &current_back_buffer);
  software_graphics_buffer_draw_rectangle_solid(&current_back_buffer, 0, 0,
  	desktop_enviroment->overlay_buffer.width, desktop_enviroment->overlay_buffer.height, desktop_background_color);
}

void desktop_enviroment_draw_overlay(Desktop_Enviroment *desktop_enviroment) {
  Color_RGB_U8 overlay_background_color = { 20, 20, 20 };
  software_graphics_buffer_draw_rectangle_solid(&desktop_enviroment->overlay_buffer, 0, 0, 
  	desktop_enviroment->overlay_buffer.width, desktop_enviroment->overlay_buffer.height, overlay_background_color);
  for (size_t i = 0; i < desktop_enviroment->search_entry_array.count; i++) {
    Search_Entry *entry = &desktop_enviroment->search_entry_array.elements[i];
    software_graphics_draw_cstring(&desktop_enviroment->overlay_buffer, &INCONSOLATA16, entry->search_string, 25, 25);  
  }
}

void desktop_enviroment_draw(Desktop_Enviroment *desktop_enviroment) {
  desktop_enviroment_draw_desktop(desktop_enviroment);
  if (desktop_enviroment->is_overlay_active) {
  	desktop_enviroment_draw_overlay(desktop_enviroment);
  	Software_Graphics_Buffer current_back_buffer = {};
  	graphics_device_get_back_buffer(desktop_enviroment->graphics_device, &current_back_buffer);
	  software_graphics_buffer_overlay_buffers(&current_back_buffer, &desktop_enviroment->overlay_buffer, 200);
  }

  graphics_device_swap_buffers(desktop_enviroment->graphics_device);
}

void desktop_enviroment_process_input(Desktop_Enviroment *desktop_enviroment, Keyboard_State *keyboard_state) {
  if (desktop_enviroment->is_overlay_active) {
    if (keyboard_state->is_key_pressed[KEYBOARD_SCANCODE1_ENTER_PRESSED]) {
      if (desktop_enviroment->search_entry_array.count != 0) {
        Search_Entry *entry = &desktop_enviroment->search_entry_array.elements[0];
        String_Buffer_128 file_path = {};
        string_buffer_128_append_cstring(&file_path, "/programs/");
        string_buffer_128_append_cstring(&file_path, "minesweeper");
        process_create_from_elf64_file_string(file_path.data, file_path.length);
      }
    }
  }

  if (keyboard_state->is_key_pressed[KEYBOARD_SCANCODE1_UP_PRESSED]) {
  	desktop_enviroment->is_overlay_active = !desktop_enviroment->is_overlay_active;
  }
}

void desktop_enviroment_update(Desktop_Enviroment *desktop_enviroment) {
  keyboard_state_update(&globals.keyboard);
  desktop_enviroment_process_input(desktop_enviroment, &globals.keyboard);
  keyboard_state_reset(&globals.keyboard);
  desktop_enviroment_draw(desktop_enviroment);
}