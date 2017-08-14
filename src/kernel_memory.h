
typedef struct {
  uintptr_t value;
} Physical_Address;

typedef struct {
  uintptr_t value;
} Virtual_Address;

typedef struct {
  uintptr_t physical_address;
  uint64_t size_in_bytes;
  uint64_t physical_4KB_page_count;
} Memory_Range;

typedef struct {
  uint64_t present : 1; //0
  uint64_t writeable : 1; // 1
  uint64_t user_accessible : 1; //2
  uint64_t write_through_caching : 1; //3 Makes writes go directly to memory
  uint64_t disable_cache : 1; //4
  //CPU sets this bit when used.  DO NOT TOUCH
  uint64_t accessed : 1; //5 
  //CPU sets this bit when touched.  DONT TOUCH
  uint64_t dirty : 1; //6
  //This must be 0 in p4 and p1.  1GB in p3, 2MB in p2
  uint64_t huge_page : 1; //7
  uint64_t global : 1; // 8
} __attribute((packed)) Page_Table_Entry;

typedef struct {
  uintptr_t entries[512];
} Page_Table;

#if 0
#include "utils/dynamic_sized_stack.h"
//Generate Free_Page_Stack data structure
DEFINE_DYNAMIC_SIZED_STACK(Free_Page_Stack, free_page_stack, uint64_t)
#endif

typedef struct {
  //Usable Memory Ranges
  //NOTE(Torin 2017-08-11) Already initialized from
  //multiboot2 information
  Memory_Range usable_ranges[8];
  uint64_t usable_range_count;
  uint64_t total_usable_memory;

  //Physical Page Allocation
  Spin_Lock physical_page_allocator_lock;
  Memory_Range *current_usable_range;
  uint64_t current_usable_range_index;
  uint64_t next_free_physical_page_index_in_current_range;
  //Free_Page_Stack free_page_stack;

  uint64_t current_kernel_persistent_virtual_memory_address;
} Kernel_Memory_State;

static const uint64_t PAGE_PRESENT_BIT            = 1L << 0;
static const uint64_t PAGE_WRITEABLE_BIT          = 1L << 1;
static const uint64_t PAGE_USER_ACCESS_BIT        = 1L << 2;
static const uint64_t PAGE_WRITE_TROUGH_CACHE_BIT = 1L << 3;
static const uint64_t PAGE_DISABLE_CACHE_BIT      = 1L << 4;
static const uint64_t PAGE_ACCESSED_BIT           = 1L << 5;
static const uint64_t PAGE_DIRY_BIT               = 1L << 6;
static const uint64_t PAGE_HUGE_BIT               = 1L << 7;
static const uint64_t PAGE_GLOBAL_BIT             = 1L << 8;
static const uint64_t PAGE_NO_EXECUTE_BIT         = 1L << 63;

//NOTE(Torin 2017-08-11) Usable ranges are added at kernel entry time
//using the multiboot2 information. Usable ranges must be added
//before memory_manager_initialize is called.


void memory_manager_initialize();


uintptr_t memory_get_physical_address(uintptr_t virtual_address);
uintptr_t memory_p4_index_of_virtual_address(uintptr_t virtual_address);
uintptr_t memory_p3_index_of_virtual_address(uintptr_t virtual_address);
uintptr_t memory_p2_index_of_virtual_address(uintptr_t virtual_address);
uintptr_t memory_p1_index_of_virtual_address(uintptr_t virtual_address);

bool memory_usable_range_contains(uintptr_t address, size_t size);
void memory_usable_range_add(uintptr_t address, uint64_t size);
bool memory_usable_range_find_containing_physical_address(uintptr_t physical_address, uint64_t *range_index);

//NOTE(Torin 2017-08-11) Used to safely allocate a single 4KB physical page
//without any consideration for contigious physical pages.
uintptr_t memory_physical_4KB_page_acquire(); 
//NOTE(Torin 2017-08-11) Does not unmap anything.  Adds page
//address to the free physical_page_stack.
void memory_physical_4KB_page_release(uintptr_t physical_page);

//NOTE(Torin 2017-08-11) Uses recursive mapping to get a virtual address
//for the page table entry at the provided index.
Page_Table *memory_get_or_create_page_table(Page_Table *table, uint64_t index);

void memory_map_physical_to_virtual(uintptr_t physical_page, uintptr_t virtual_address);
void memory_unmap_virtual_address(uintptr_t virtual_address);

uint8_t *memory_allocate_persistent_virtual_pages(uint64_t page_count);

uintptr_t memory_map_physical_mmio(uintptr_t physical_address, uint64_t page_count);
void memory_tlb_flush();