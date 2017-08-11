
typedef struct {
  uintptr_t value;
} Physical_Address;

typedef struct {
  uintptr_t value;
} Virtual_Address;

typedef enum {
  Memory_Range_Type_KERNEL;
  Memory_Range_Type_USABLE,
  Memory_Range_Type_UNUSABLE
} Memory_Range_Type;

typedef struct {
  uintptr_t address;
  uint64_t size;
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

//Generate Free_Page_Stack data structure
#define DYNAMIC_SIZED_STACK_NAME Free_Page_Stack
#define DYNAMIC_SIZED_STACK_PREFIX free_page_stack
#define DYNAMIC_SIZED_STACK_ELEMENT uint64_t
#include "utils/dynamic_sized_stack.h"


typedef struct {
  //Usable Memory Ranges
  //NOTE(Torin 2017-08-11) Already initialized from
  //multiboot2 information
  Memory_Range usable_range[8];
  uint64_t usable_range_count;
  uint64_t total_usable_memory;

  //Physical Page Allocation
  Memory_Range *current_usable_range;
  uint64_t current_usable_range_index;
  uint64_t next_free_physical_page_index_in_current_range;
  Free_Page_Stack free_page_stack;

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

uintptr_t kmem_map_physical_mmio(Kernel_Memory_State *memstate, uintptr_t physical_address, uint64_t page_count);
bool kmem_allocate_physical_pages(Kernel_Memory_State *memstate, uint64_t page_count, uintptr_t *out);
uintptr_t kmem_allocate_persistant_kernel_memory(Kernel_Memory_State *memstate, uint64_t page_count);
uintptr_t kmem_push_temporary_kernel_memory(uintptr_t physical_address);
void kmem_pop_temporary_kernel_memory();

//NOTE(Torin) Temporary debug bullshit
//===============================================================
void kmem_map_physical_to_virtual_unaccounted(Kernel_Memory_State *memstate, uintptr_t physical_address, uintptr_t virtual_address, uint64_t flags);
uintptr_t kmem_map_unaligned_physical_to_aligned_virtual_unaccounted(Kernel_Memory_State *memstate, uintptr_t requested_physical_address, uintptr_t virtual_address, uint64_t flags);
