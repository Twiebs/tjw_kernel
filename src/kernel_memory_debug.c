
void memory_debug_log_virtual_address_info_4KB(const uintptr_t virtual_address) {
  uintptr_t p4_index = memory_p4_index_of_virtual_address(virtual_address);
  uintptr_t p3_index = memory_p3_index_of_virtual_address(virtual_address);
  uintptr_t p2_index = memory_p2_index_of_virtual_address(virtual_address);
  uintptr_t p1_index = memory_p1_index_of_virtual_address(virtual_address);
  uint64_t offset = virtual_address & 0xFFF;

  klog_debug("virtual_address_info:");
  klog_debug("  virtual_address: 0x%X", virtual_address);
  klog_debug("  p4_index: %lu", p4_index);
  klog_debug("  p3_index: %lu", p3_index);
  klog_debug("  p2_index: %lu", p2_index);
  klog_debug("  p1_index: %lu", p1_index);
  klog_debug("  offset: %lu", offset);

  klog_debug("p4_entry_info:");
  kdebug_log_page_table_entry_info(g_p4_table.entries[p4_index]);
  klog_debug("p3_entry_info:");
  kdebug_log_page_table_entry_info(g_p3_table.entries[p3_index]);
  klog_debug("p2_entry_info:");
  kdebug_log_page_table_entry_info(g_p2_table.entries[p2_index]);
  klog_debug("p1_entry_info:");
  kdebug_log_page_table_entry_info(g_p2_table.entries[p2_index]);
}

void kdebug_log_page_table_entry_info(const uintptr_t entry) {
  uintptr_t physical_address = entry & ~0xFFF;
  bool is_present = entry & PAGE_PRESENT_BIT;
  bool is_writeable = entry & PAGE_WRITEABLE_BIT;
  bool access_mode = entry & PAGE_USER_ACCESS_BIT;
  bool is_write_through_caching = entry & PAGE_WRITE_TROUGH_CACHE_BIT;
  bool is_caching_disabled = entry & PAGE_DISABLE_CACHE_BIT;
  bool is_huge_page = entry & PAGE_HUGE_BIT;
  klog_debug("  physical_address: 0x%X", physical_address);
  klog_debug("  present: %s", is_present ? "true" : "false");
  klog_debug("  writeable: %s", is_writeable ? "true" : "false");
  klog_debug("  user accessiable: %s", access_mode ? "true" : "false");
  klog_debug("  write through caching: %s", is_write_through_caching ? "true" : "false");
  klog_debug("  caching disabled: %s", is_caching_disabled ? "true" : "false");
  klog_debug("  huge page: %s", is_huge_page ? "true" : "false"); 
}

#if 0
void kdebug_log_virtual_address_info_2MB(const uintptr_t virtual_address) {
  uint64_t p4_index = (virtual_address >> 39) & 0x1FF;
  uint64_t p3_index = (virtual_address >> 30) & 0x1FF;
  uint64_t p2_index = (virtual_address >> 21) & 0x1FF;
  uint64_t offset   = (virtual_address >> 0)  & 0xFFFFF;

  klog_debug("logging virtual address info");
  klog_debug(" virtual_address: 0x%X", virtual_address);
  klog_debug(" p4_index: %lu", p4_index);
  klog_debug(" p3_index: %lu", p3_index);
  klog_debug(" p2_index: %lu", p2_index);
  klog_debug(" offset: %lu", offset);

  klog_debug(" p4_entry_info:");
  kdebug_log_page_table_entry_info(g_p4_table.entries[p4_index]);
  klog_debug(" p3_entry_info:");
  kdebug_log_page_table_entry_info(g_p3_table.entries[p3_index]);
  klog_debug(" p2_entry_info:");
  kdebug_log_page_table_entry_info(g_p2_table.entries[p2_index]);
}


void kdebug_kmem_log_memory_state(Kernel_Memory_State *mem) {
  klog_debug("Kernel_Memory_State:");
  uint64_t total_usable_memory_mb = mem->total_usable_memory / (1024*1024);
  klog_debug("  total_usable_memory: %luMB", total_usable_memory_mb);
  for (size_t i = 0; i < mem->usable_range_count; i++) {
    Memory_Range *range = &mem->usable_range[i];
    uintptr_t range_end = range->address + range->size;
    uint64_t range_size_mb = range->size / (1024*1024);
    klog_debug("  0x%X - 0x%X: %luMB", range->address, range_end, range_size_mb);
  }
}
#endif