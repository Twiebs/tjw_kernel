
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
  if (vfs_acquire_node_handle(shell->current_directory, shell->current_directory_count, &handle)) {
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
  if (vfs_acquire_node_handle(path, path_length, &handle)) {
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
  
  log_debug(DEFAULT, "%s", temporary_buffer);
}

void shell_command_lspci(Shell_Command_Parameter_Info *parameter_info) {
  System_Info *system = &globals.system_info;
  for (size_t i = 0; i < system->pci_device_count; i++) {
    PCI_Device *pci_device = &system->pci_devices[i];
    klog_info("%u:%u:%u %s subclass: 0x%X, progif: 0x%X, vendor: 0x%X, device: 0x%X", pci_device->bus_number, 
      pci_device->device_number, pci_device->function_number, pci_device->type_description,
      pci_device->subclass, pci_device->programming_interface, pci_device->vendor_id, pci_device->device_id);
  }

  for (size_t i = 0; i < system->pci_device_count; i++) {
    PCI_Device *pci_device = &system->pci_devices[i];
    pci_debug_log_pci_device(pci_device);
  }

}

void shell_command_run(Shell_Command_Parameter_Info *parameter_info) {
  Shell_Command_Parameter *path_param = &parameter_info->parameters[0];
  const char *path = path_param->text;
  size_t path_length = path_param->length;
  process_create_from_elf64_file_string(path, path_length);
}

void shell_command_version(Shell_Command_Parameter_Info *parameter_info)
{
  klog_info("Kernel Version %u.%u.%u", KERNEL_VERSION_MAJOR, KERNEL_VERSION_MINOR, KERNEL_VERSION_REVISION);
}

void shell_command_lazy(Shell_Command_Parameter_Info *parameter_info) {
  process_create_from_elf64_file_string("/programs/minesweeper", sizeof("/programs/minesweeper") - 1);
}