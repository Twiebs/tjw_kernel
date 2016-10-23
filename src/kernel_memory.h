
typedef struct {
  uintptr_t address;
  uint64_t size;
} Memory_Range;

typedef struct {
  Memory_Range usable_range[8];
  uint64_t usable_range_count;
  uint64_t total_usable_memory;
  uintptr_t allocator_start;
  uint32_t current_page_index;

  uint64_t *page_usage_bitmap;
  uint64_t page_usage_bitmap_size;
  uint64_t page_usage_bitmap_page_count;

  //NOTE(Torin) The kernel is mapped into a linear virtual address space
  uintptr_t kernel_memory_start_virtual_address;
  uint64_t kernel_memory_used_page_count;
  uint64_t kernel_memory_allocated_page_table_count;
} Kernel_Memory_State;

uintptr_t kmem_map_physical_mmio(Kernel_Memory_State *memstate, uintptr_t physical_address, uint64_t page_count);
uintptr_t kmem_allocate_persistant_kernel_memory(Kernel_Memory_State *memstate, uint64_t page_count);

//NOTE(Torin) Temporary debug bullshit
//===============================================================
void kmem_map_physical_to_virtual_unaccounted(Kernel_Memory_State *memstate, uintptr_t physical_address, uintptr_t virtual_address, uint64_t flags);
uintptr_t kmem_map_unaligned_physical_to_aligned_virtual_unaccounted(Kernel_Memory_State *memstate, uintptr_t requested_physical_address, uintptr_t virtual_address, uint64_t flags);
