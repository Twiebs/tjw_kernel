
void shell_command_help(Shell_Command_Parameter_Info *parameter_info) {
  Command_Line_Shell *shell = &globals.shell;
  for (size_t i = 0; i < shell->command_count; i++) {
    Shell_Command *command = &shell->commands[i];
    klog_info("- %s", command->name);
  }
}

void print_directory_entries(VFS_Node_Info *info, void *userdata) {
  klog_info("%s", info->name);
}

void shell_command_ls(Shell_Command_Parameter_Info *parameter_info) {
  Command_Line_Shell *shell = &globals.shell;
  VFS_Node_Handle handle = {};
  if (vfs_aquire_node_handle(shell->current_directory, shell->current_directory_count, &handle)) {
    klog_error("ls: failed to get handle %.*s", (int)shell->current_directory_count, shell->current_directory);
    return;
  }

  vfs_node_iterate_directory(&handle, print_directory_entries, NULL);
}

void shell_command_cd(Shell_Command_Parameter_Info *parameter_info) {
  Command_Line_Shell *shell = &globals.shell;
  Shell_Command_Parameter *directory = &parameter_info->parameters[0];
  if (directory->text[0] == '/') {
    memory_copy(shell->current_directory, directory->text, directory->length);
    shell->current_directory_count = directory->length;
  } else {
    memory_copy(&shell->current_directory[shell->current_directory_count], directory->text, directory->length);
    shell->current_directory_count += directory->length;
    shell->current_directory[shell->current_directory_count++] = '/';
  }
}

void shell_command_cat(Shell_Command_Parameter_Info *parameter_info) {
  Command_Line_Shell *shell = &globals.shell;
  Shell_Command_Parameter *path_param = &parameter_info->parameters[0];
  const char *path = path_param->text;
  size_t path_length = path_param->length;
  uint8_t *temporary_buffer = cpu_get_temporary_memory();
  if (path_param->text[0] != '/') {

  }

  VFS_Node_Handle handle = {};
  if (vfs_aquire_node_handle(path, path_length, &handle)) {
    klog_error("cat: failed to get handle %.*s", (int)path_length, path);
    return;
  }

  //TODO(Torin: 2017-08-06) Really need temporary memory!
  //TODO(Torin: 2017-08-06) Switch to virtual memory file reads!
  size_t file_size = min(handle.file_size, 4096);
  if (vfs_node_read_file(&handle, 0, handle.file_size, temporary_buffer)) {
    klog_error("cat: failed file read");
    return;
  }
  
  klog_debug("%s", temporary_buffer);
}
