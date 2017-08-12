

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