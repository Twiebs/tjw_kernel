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

static uint32_t g_current_page_index;
static uint32_t g_p1_table_count;
static PageTable g_p1_table __attribute__((aligned(4096)));

//Page table entry configuration bits
static const uint64_t PAGE_PRESENT_BIT 		        = 1L << 0;
static const uint64_t PAGE_WRITEABLE_BIT 	        = 1L << 1;
static const uint64_t PAGE_USER_ACCESS_BIT        = 1L << 2;
static const uint64_t PAGE_WRITE_TROUGH_CACHE_BIT = 1L << 3;
static const uint64_t PAGE_DISABLE_CACHE_BIT      = 1L << 4;
static const uint64_t PAGE_ACCESSED_BIT           = 1L << 5;
static const uint64_t PAGE_DIRY_BIT               = 1L << 6;
static const uint64_t PAGE_HUGE_BIT 			        = 1L << 7;
static const uint64_t PAGE_GLOBAL_BIT             = 1L << 8;
static const uint64_t PAGE_NO_EXECUTE_BIT         = 1L << 63;

//=============================================================================================

void kmem_map_physical_to_virtual_2MB_ext(uintptr_t physical_address, uintptr_t virtual_address, uint64_t flags){
  kassert((virtual_address & 0x1FFFFF) == 0);
  kassert((physical_address & 0x1FFFFF) == 0);
  uintptr_t p4_index = (virtual_address >> 39) & 0x1FF;
  uintptr_t p3_index = (virtual_address >> 30) & 0x1FF;
  uintptr_t p2_index = (virtual_address >> 21) & 0x1FF;
  PageTable *p4_table = (PageTable *)&g_p4_table;
  PageTable *p3_table = (PageTable *)p4_table->entries[p4_index]; 
  PageTable *p2_table = (PageTable *)p3_table->entries[p3_index];
  g_p2_table.entries[p2_index] = physical_address | PAGE_PRESENT_BIT | PAGE_HUGE_BIT | PAGE_WRITEABLE_BIT | flags;
}

void kmem_map_physical_to_virtual_2MB(uintptr_t physical_address, uintptr_t virtual_address){
  kassert((virtual_address & 0x1FFFFF) == 0);
  kassert((physical_address & 0x1FFFFF) == 0);
  uintptr_t p4_index = (virtual_address >> 39) & 0x1FF;
  uintptr_t p3_index = (virtual_address >> 30) & 0x1FF;
  uintptr_t p2_index = (virtual_address >> 21) & 0x1FF;
  kassert(p4_index == 0 && p3_index == 0);
  kassert(g_p2_table.entries[p2_index] == 0);
  g_p2_table.entries[p2_index] = physical_address | PAGE_PRESENT_BIT | PAGE_HUGE_BIT | PAGE_WRITEABLE_BIT;
}

uintptr_t kmem_map_unaligned_physical_to_aligned_virtual_2MB(uintptr_t requested_physical_address, uintptr_t virtual_address){
  uint64_t physical_address_to_map = requested_physical_address;
	uint64_t displacement_from_page_boundray = requested_physical_address & 0x1FFFFF;
  physical_address_to_map -= displacement_from_page_boundray;
  kmem_map_physical_to_virtual_2MB(physical_address_to_map, virtual_address);
  return displacement_from_page_boundray;
}

void kmem_initalize(){
  g_p2_table.entries[1] = (uintptr_t)&g_p1_table.entries[0] | PAGE_WRITEABLE_BIT | PAGE_PRESENT_BIT;
	g_current_page_index = 2;
}

//=============================================================================================

#if 0
static uintptr_t //virtual_page_address
silly_page_map_4KB(uintptr_t physical_page_address){
  g_p1_table.entries[g_p1_table_count] = physical_page_address | PAGE_PRESENT_BIT | PAGE_WRITEABLE_BIT; 
  uintptr_t result = (1024*1024*2) + (g_p1_table_count * 4096);
  return result;
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
#endif
