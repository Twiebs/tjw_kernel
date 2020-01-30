//NOTE(Torin) Some conventions used in the kernel
//p4 is the PML4E
//p3 is the PDPE
//p2 is the PDE
//p1 is the PTE

extern Page_Table g_p4_table;
extern Page_Table g_p3_table;
extern Page_Table g_p2_table;


static inline void kmem_clear_page(void *page) {
  memory_set(page, 0x00, 4096);
}

Page_Table *memory_get_page_table_address(Page_Table *table, uint64_t index) {
  uintptr_t next_table_address = ((uintptr_t)table << 9) | (index << 12);
  Page_Table *result = (Page_Table *)next_table_address;
  return result;
}

Physical_Address memory_get_physical_address(Virtual_Address virtual_address) {
  uintptr_t p4_index = memory_p4_index_of_virtual_address(virtual_address);
  uintptr_t p3_index = memory_p3_index_of_virtual_address(virtual_address);
  uintptr_t p2_index = memory_p2_index_of_virtual_address(virtual_address);
  uintptr_t p1_index = memory_p1_index_of_virtual_address(virtual_address);
  Page_Table *p4_table = (Page_Table *)0xFFFFFFFFFFFFF000;
  Page_Table *p3_table = memory_get_page_table_address(p4_table, p4_index);
  Page_Table *p2_table = memory_get_page_table_address(p3_table, p3_index);
  Page_Table *p1_table = memory_get_page_table_address(p2_table, p2_index);
  uintptr_t physical_address = p1_table->entries[p1_index] & ~0xFFF;
  physical_address += virtual_address & 0xFFF;
  return physical_address;
}

uintptr_t memory_p4_index_of_virtual_address(uintptr_t virtual_address) {
  uintptr_t p4_index = (virtual_address >> 39) & 0x1FF;
  return p4_index;
}

uintptr_t memory_p3_index_of_virtual_address(uintptr_t virtual_address) {
  uintptr_t p3_index = (virtual_address >> 30) & 0x1FF;
  return p3_index;
}

uintptr_t memory_p2_index_of_virtual_address(uintptr_t virtual_address) {
  uintptr_t p2_index = (virtual_address >> 21) & 0x1FF;
  return p2_index;
}

uintptr_t memory_p1_index_of_virtual_address(uintptr_t virtual_address) {
  uintptr_t p1_index = (virtual_address >> 12) & 0x1FF;
  return p1_index;
}

void memory_tlb_flush() {
  asm volatile(".intel_syntax noprefix");
  asm volatile("mov rax, cr3");
  asm volatile("mov cr3, rax");
  asm volatile(".att_syntax prefix");
}

static uint8_t temporary_block[4096];
uint8_t *reserve_temporary_block() {
  return temporary_block;
}


Page_Table *memory_get_or_create_page_table(Page_Table *table, uint64_t index) {
  if ((table->entries[index] & PAGE_PRESENT_BIT) == 0) {
    uintptr_t physical_page = memory_physical_4KB_page_acquire();
    table->entries[index] = physical_page;
    table->entries[index] |= PAGE_PRESENT_BIT;
    table->entries[index] |= PAGE_WRITEABLE_BIT;
    uintptr_t page_address = ((uintptr_t)table << 9) | (index << 12);
    memory_set((void *)page_address, 0x00, 4096);
  }

  uintptr_t next_table_address = ((uintptr_t)table << 9) | (index << 12);
  Page_Table *result = (Page_Table *)next_table_address;
  return result;
}

bool memory_usable_range_find_containing_physical_address(uintptr_t physical_address, uint64_t *range_index) {
  Kernel_Memory_State *memory = &globals.memory_state;
  for (size_t i = 0; i < memory->usable_range_count; i++) {
    Memory_Range *range = &memory->usable_ranges[i];
    if ((physical_address >= range->physical_address) && (physical_address <= range->physical_address + range->size_in_bytes)) {
      if (range_index != NULL) *range_index = i;
      return true;
    }
  }
  return false;
}

bool memory_usable_range_contains(uintptr_t address, size_t size) {
  Kernel_Memory_State *memory = &globals.memory_state;
  uintptr_t target_end = address + size;
  for (size_t i = 0; i < memory->usable_range_count; i++) {
    Memory_Range *range = &memory->usable_ranges[i];
    uintptr_t range_end = range->physical_address + range->size_in_bytes;
    if ((address >= range->physical_address) && (target_end <= range_end)) return true;
  }
  return false;
}

void memory_usable_range_add(uintptr_t address, uint64_t size) {
  Kernel_Memory_State *memory = &globals.memory_state;
  if (memory->usable_range_count + 1 > ARRAY_COUNT(memory->usable_ranges)) {
    uintptr_t range_end = address + size;
    klog_warning("maximum memory range entries was execeded.  Memory_Range 0x%X - 0x%X is unusable", address, range_end);
    return;
  }

  memory->usable_ranges[memory->usable_range_count].physical_address = address;
  memory->usable_ranges[memory->usable_range_count].size_in_bytes = size;
  memory->usable_ranges[memory->usable_range_count].physical_4KB_page_count = size / 4096;
  memory->usable_range_count++;
  memory->total_usable_memory += size;
}

//NOTE(Torin 2017-08-11) Used to safely allocate a single 4KB physical page
//without any consideration for multipule physical page address consistancy
uintptr_t memory_physical_4KB_page_acquire() {
  Kernel_Memory_State *memory = &globals.memory_state;
  spin_lock_acquire(&memory->physical_page_allocator_lock);
  kassert(memory->current_usable_range != NULL);
  //NOTE(Torin 2017-08-11) If there are no more free pages in current_usable_range
  //we seek through the usable_range array and find the next range.  If the
  //system is out of ranges there is no more physical memory to allocate.
  if (memory->next_free_physical_page_index_in_current_range + 1 > 
      memory->current_usable_range->physical_4KB_page_count) {
    size_t usable_range_index = memory->current_usable_range_index + 1;
    Memory_Range *next_usable_range = memory->current_usable_range;
    //NOTE(Torin 2017-08-11) This is wrriten strangely so non-usuable ranges can be added.
    while (usable_range_index < memory->usable_range_count) {
      Memory_Range *range = &memory->usable_ranges[usable_range_index];
      next_usable_range = range;
      memory->current_usable_range_index = usable_range_index;
      memory->next_free_physical_page_index_in_current_range = 0;
      break;
    }

    if (next_usable_range == memory->current_usable_range) {
      spin_lock_release(&memory->physical_page_allocator_lock);
      klog_error("OUT OF MEMORY");
      kernel_panic();
    }
  }

  uintptr_t result = memory->current_usable_range->physical_address + 
    (memory->next_free_physical_page_index_in_current_range * 4096);
  memory->next_free_physical_page_index_in_current_range += 1;
  spin_lock_release(&memory->physical_page_allocator_lock);
  //klog_debug("[Memory] Allocated physical page: 0x%X", result);
  return result;
}

void memory_physical_4KB_page_release(uintptr_t physical_page) {

}

void memory_map_physical_to_virtual(uintptr_t physical_page, uintptr_t virtual_address) {
  if (physical_page & 0xFFF) {
    klog_error("cannot map physical_page: 0x%X to virtual_address: 0x%X physical_page is unaligned", physical_page, virtual_address);
    kassert(false);
    return;
  }

  uint64_t p4_index = memory_p4_index_of_virtual_address(virtual_address);
  uint64_t p3_index = memory_p3_index_of_virtual_address(virtual_address);
  uint64_t p2_index = memory_p2_index_of_virtual_address(virtual_address);
  uint64_t p1_index = memory_p1_index_of_virtual_address(virtual_address);
  Page_Table *p4_table = (Page_Table *)0xFFFFFFFFFFFFF000;
  Page_Table *p3_table = memory_get_or_create_page_table(p4_table, p4_index);
  Page_Table *p2_table = memory_get_or_create_page_table(p3_table, p3_index);
  Page_Table *p1_table = memory_get_or_create_page_table(p2_table, p2_index);
  if (p1_table->entries[p1_index] & PAGE_PRESENT_BIT) {
    klog_error("cannot map physical_page: 0x%X to virtual_address: 0x%X", physical_page, virtual_address);
    kernel_panic();
    return;
  }

  p1_table->entries[p1_index] = physical_page;
  p1_table->entries[p1_index] |= PAGE_PRESENT_BIT;
  p1_table->entries[p1_index] |= PAGE_WRITEABLE_BIT;
  memory_tlb_flush();
  //klog_debug("[Memory] Mapped physical page: 0x%X to virtual address: 0x%X", physical_page, virtual_address);
}

void memory_unmap_virtual_address(uintptr_t virtual_address) {
  uint64_t p4_index = memory_p4_index_of_virtual_address(virtual_address);
  uint64_t p3_index = memory_p3_index_of_virtual_address(virtual_address);
  uint64_t p2_index = memory_p2_index_of_virtual_address(virtual_address);
  uint64_t p1_index = memory_p1_index_of_virtual_address(virtual_address);
  Page_Table *p4_table = (Page_Table *)0xFFFFFFFFFFFFF000;
  Page_Table *p3_table = memory_get_or_create_page_table(p4_table, p4_index);
  Page_Table *p2_table = memory_get_or_create_page_table(p3_table, p3_index);
  Page_Table *p1_table = memory_get_or_create_page_table(p2_table, p2_index);
  if ((p1_table->entries[p1_index] & PAGE_PRESENT_BIT) == 0) {
    klog_error("[Memory] Can not unmap virtual_address: 0x%X The address is not mapped", virtual_address);
    kassert(false);
    return;
    
  }

  uintptr_t physical_page = p1_table->entries[p1_index] & ~0xFFF;
  p1_table->entries[p1_index] = 0;
  memory_tlb_flush();
  log_debug(MEMORY, "Unmapped physical page: 0x%X from virtual address: 0x%X", physical_page, virtual_address);
}

uint8_t *memory_allocate_persistent_virtual_pages(uint64_t page_count) {
  Kernel_Memory_State *memory = &globals.memory_state;
  uint8_t *result = (uint8_t *)memory->current_kernel_persistent_virtual_memory_address;
  for (size_t i = 0; i < page_count; i++) {
    uintptr_t physical_page = memory_physical_4KB_page_acquire();
    memory_map_physical_to_virtual(physical_page, memory->current_kernel_persistent_virtual_memory_address + (i * 4096));
  }

  memory->current_kernel_persistent_virtual_memory_address += page_count * 4096;
  return result;
}

void memory_manager_initialize(Primary_CPU_Initialization_Info *initialization_info) 
{
    kassert(initialization_info);
    kassert(initialization_info->usable_range_count > 0);

    Kernel_Memory_State *memory = &globals.memory_state;
    kassert(memory->usable_range_count == 0);

    // NOTE(Torin, 2020-01-17) Primary_CPU_Initialization_Info Contains the usable memory ranges
    // available to the memory manager. This information is typically extracted from the multiboot2 info.
    memory->usable_range_count=initialization_info->usable_range_count;
    for (size_t i = 0; i < initialization_info->usable_range_count; i++)
    {
        memory->usable_ranges[i] = initialization_info->usable_ranges[i];
    }
    




  //NOTE(Torin 2017-08-11) Mark recursive map present
  g_p4_table.entries[511] |= PAGE_PRESENT_BIT | PAGE_WRITEABLE_BIT;

  extern uint32_t _KERNEL_END;
  globals.system_info.kernel_end = 0x100000 + _KERNEL_END;
  globals.system_info.kernel_end = memory_align(globals.system_info.kernel_end, 0x200000);

  uint64_t memory_range_index = 0;
  if (memory_usable_range_find_containing_physical_address(globals.system_info.kernel_end, &memory_range_index) == false) {
    klog_error("WTF MEMORY RANGES DONT CONTAIN KERNEL END?");
    return;
  }

  memory->current_kernel_persistent_virtual_memory_address = globals.system_info.kernel_end;

  Memory_Range *range = &memory->usable_ranges[memory_range_index];
  uint64_t offset_into_range = globals.system_info.kernel_end - range->physical_address;
  memory->current_usable_range_index = memory_range_index;
  kassert((offset_into_range % 4096) == 0);
  memory->next_free_physical_page_index_in_current_range = offset_into_range / 4096;
  memory->current_usable_range = range;
  log_info(MEMORY, "Kernel Memory Manager initalized");
}

uintptr_t memory_map_physical_mmio(uintptr_t physical_address, uint64_t page_count) {
  Kernel_Memory_State *memory = &globals.memory_state;
  kassert((physical_address & 0xFFF) == 0);

  //TODO(Torin 2017-08-11) This does 
  if (memory_usable_range_contains(physical_address, page_count*4096)) {
    klog_error("cannot map physical address for MMIO because it is contained within the usable range of memory");
    kernel_panic();
  }

  uintptr_t result = memory->current_kernel_persistent_virtual_memory_address;
  for (size_t i = 0; i < page_count; i++) {
    memory_map_physical_to_virtual(physical_address + (i*4096), result + (i*4096));
  }

  memory->current_kernel_persistent_virtual_memory_address += page_count * 4096;
  return result;
}