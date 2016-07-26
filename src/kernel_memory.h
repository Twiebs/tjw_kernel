
static uintptr_t silly_page_map(const uintptr_t requested_physical_address, const bool is_writeable, 
  uintptr_t *physical_page_address, uintptr_t *page_offset);
static void print_virtual_address_info_2MB(const uintptr_t virtual_address);