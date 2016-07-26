//NOTE(Torin) Some conventions used in the kernel
//p4 is the PML4E
//p3 is the PDPE
//p2 is the PDE
//p1 is the PTE

typedef struct {
	uintptr_t entries[512];
} PageTable;

typedef struct {
  uint32_t current_page_index;
} Kernel_Memory_State;

extern PageTable g_p4_table;
extern PageTable g_p3_table;
extern PageTable g_p2_table;
global_variable uint32_t g_current_page_index;

//static const uint64_t PAGE_NO_EXECUTE_BIT         = 1 << 63;
static const uint64_t PAGE_PRESENT_BIT 		        = 1 << 0;
static const uint64_t PAGE_WRITEABLE_BIT 	        = 1 << 1;
static const uint64_t PAGE_USER_ACCESS_BIT        = 1 << 2;
static const uint64_t PAGE_WRITE_TROUGH_CACHE_BIT = 1 << 3;
static const uint64_t PAGE_DISABLE_CACHE_BIT      = 1 << 4;
static const uint64_t PAGE_ACCESSED_BIT           = 1 << 5;
static const uint64_t PAGE_DIRY_BIT               = 1 << 6;
static const uint64_t PAGE_HUGE_BIT 			        = 1 << 7;
static const uint64_t PAGE_GLOBAL_BIT             = 1 << 8;

static void
log_page_info()
{
  for(size_t i = 0; i < g_current_page_index; i++){
    bool is_present = g_p2_table.entries[i] & 0b01;
    uintptr_t physical_address = g_p2_table.entries[i] & ~(0b111111111111);
    uintptr_t virtual_address = i * 1024 * 1024 * 2;
    klog_debug("page_entry: virtual %lu mapped to %lu", virtual_address, physical_address);
  }
}

static void
print_page_table_entry_info(const uintptr_t entry)
{
  uintptr_t physical_address = entry & ~0xFFF;
  bool is_present = entry & PAGE_PRESENT_BIT;
  bool is_writeable = entry & PAGE_WRITEABLE_BIT;
  bool access_mode = entry & PAGE_USER_ACCESS_BIT;
  bool is_write_through_caching = entry & PAGE_WRITE_TROUGH_CACHE_BIT;
  bool is_caching_disabled = entry & PAGE_DISABLE_CACHE_BIT;
  bool is_huge_page = entry & PAGE_HUGE_BIT;

  klog_debug("physical_address: %lu", physical_address);
  klog_debug("present: %s", is_present ? "true" : "false");
  klog_debug("writeable: %s", is_writeable ? "true" : "false");
  klog_debug("user accessiable: %s", access_mode ? "true" : "false");
  klog_debug("write through caching: %s", is_write_through_caching ? "true" : "false");
  klog_debug("caching disabled: %s", is_caching_disabled ? "true" : "false");
  klog_debug("huge page: %s", is_huge_page ? "true" : "false"); 
}

static void 
print_virtual_address_info_2MB(const uintptr_t virtual_address)
{
  uint64_t p4_index = (virtual_address >> 39) & 0x1FF;
  uint64_t p3_index = (virtual_address >> 30) & 0x1FF;
  uint64_t p2_index = (virtual_address >> 21) & 0x1FF;
  uint64_t offset   = (virtual_address >> 0)  & 0xFFFFF;

  klog_debug("virtual_address: %lu", virtual_address);
  klog_debug("p4_index: %lu", p4_index);
  klog_debug("p3_index: %lu", p3_index);
  klog_debug("p2_index: %lu", p2_index);
  klog_debug("offset: %lu", offset);

  klog_debug("p4_entry_info:");
  print_page_table_entry_info(g_p4_table.entries[p4_index]);
  klog_debug("p3_entry_info:");
  print_page_table_entry_info(g_p3_table.entries[p3_index]);
  klog_debug("p2_entry_info:");
  print_page_table_entry_info(g_p2_table.entries[p2_index]);
}

//NOTE(Torin) A simple and dirty page map that maps an entire
//2MB physical memory block into the virtual address space
static uintptr_t //virtual_page_address
silly_page_map(const uintptr_t requested_physical_address, const bool is_writeable, 
  uintptr_t *physical_page_address, uintptr_t *page_offset)
{
  //TODO(Torin) Make sure pages default to using the NOEXECUTE bit and have a seperate mechanisim
  //inplace for when ELF executables are loaded so that allocated memory can never be executed from

  //NOTE(Torin) Bits 9 - 11 and Bits 52-62 are can be used freely
  //NOTE(Torin) if requested_physical_address is not aligned to a page boundray 
  //the resulting virtual_address is aligned to nearest page boundray and the offset into
  //the result virtual_address is set to the offset output parameter

  //NOTE(Torin) 2MB pages must be aligned on a 2MB boundray not 4KB	
  uint64_t physical_address_to_map = requested_physical_address;
	uint64_t displacement_from_page_boundray = requested_physical_address & 0x1FFFFF;
  physical_address_to_map -= displacement_from_page_boundray;

	g_p2_table.entries[g_current_page_index] = physical_address_to_map | PAGE_PRESENT_BIT | PAGE_HUGE_BIT;
  if(is_writeable) g_p2_table.entries[g_current_page_index] |= PAGE_WRITEABLE_BIT;

  uintptr_t mapped_virtual_address = g_current_page_index * 1024 * 1024 * 2;
	*page_offset = displacement_from_page_boundray;
  *physical_page_address = physical_address_to_map;
	g_current_page_index += 1;

	klog_debug("[kmem] page was allocated at physical_address %lu to map to virtual address %lu, "
    "the offset from the actual requested physical_addresss(%lu) is %lu, "
    "the page table entry is %lu",
    physical_address_to_map, mapped_virtual_address, requested_physical_address, 
    *page_offset, g_p2_table.entries[g_current_page_index]);
	return mapped_virtual_address;
}

static inline
void kmem_initalize(){
	g_current_page_index = 1;
	
  klog_debug("p4_table is at addr: %lu", &g_p4_table);
	klog_debug("p3_table is at addr: %lu", &g_p3_table);
	klog_debug("p2_table is at addr: %lu", &g_p2_table);
}
