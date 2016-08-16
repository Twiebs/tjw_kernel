
#if 0
static uintptr_t silly_page_map(const uintptr_t requested_physical_address, const bool is_writeable, 
  uintptr_t *physical_page_address, uintptr_t *page_offset);

static void print_virtual_address_info_2MB(const uintptr_t virtual_address);
#endif

void kmem_map_physical_to_virtual_2MB(uintptr_t physical_address, uintptr_t virtual_address);
uintptr_t kmem_map_unaligned_physical_to_aligned_virtual_2MB(uintptr_t requested_physical_address, uintptr_t virtual_address);