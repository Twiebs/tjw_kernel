

uint32_t cpu_get_id() {
  System_Info *system = &globals.system_info;
  uint32_t lapic_id = lapic_get_id(globals.system_info.lapic_virtual_address);
  for (size_t i = 0; i < system->total_cpu_count; i++) {
    if (system->cpu_lapic_ids[i] == lapic_id) { return i; }
  }

  klog_error("UNREGISTER LAPIC ID WAS USED TO GFT CPU ID");
  kernel_panic();
  return 0;
}

CPU_Info *cpu_get_info() {
  System_Info *system = &globals.system_info;
  uint32_t id = cpu_get_id();
  CPU_Info *result = &system->cpu_infos[id];
  return result;
}

uint8_t *cpu_get_temporary_memory() {
  CPU_Info *info = cpu_get_info();
  return info->temporary_memory;
}

void silly_breakpoint() {
  klog_debug("[Debug] Breakpoint hit (Press F1 to continue)");
  shell_draw_if_required(&globals.shell, &globals.log);
  bool paused = true;
  while (paused) {    
    keyboard_state_update(&globals.keyboard);
    if (globals.keyboard.is_key_pressed[Keyboard_Keycode_F1]) paused = false;
    shell_process_keyboard_input(&globals.shell, &globals.keyboard);
    shell_draw_if_required(&globals.shell, &globals.log);
    keyboard_state_reset(&globals.keyboard);
  }
}

uint8_t *system_allocate_persistent_miscellaneous_device(size_t size) {
  System_Info *system = &globals.system_info;
  spin_lock_acquire(&system->miscellaneous_device_allocator_lock);
  uint8_t *result = persistent_block_allocator_allocate(&system->miscellaneous_device_allocator, size, 16);
  spin_lock_release(&system->miscellaneous_device_allocator_lock);
  klog_debug("Allocated persistent miscellaneous device: size(%lu), address(0x%X)", size, result);
  return result;
}