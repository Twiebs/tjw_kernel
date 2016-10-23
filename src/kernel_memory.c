//NOTE(Torin) Some conventions used in the kernel
//p4 is the PML4E
//p3 is the PDPE
//p2 is the PDE
//p1 is the PTE

typedef struct {
	uintptr_t entries[512];
} Page_Table;

extern Page_Table g_p4_table;
extern Page_Table g_p3_table;
extern Page_Table g_p2_table;

#if 0
static uint32_t g_current_page_index;
static uint32_t g_p1_table_count;
static Page_Table g_p1_table __attribute__((aligned(4096)));
#endif

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
static inline
void kdebug_kmem_log_memory_state(Kernel_Memory_State *mem){
  klog_debug("Kernel_Memory_State:");
  uint64_t total_usable_memory_mb = mem->total_usable_memory / (1024*1024);
  klog_debug("  total_usable_memory: %luMB", total_usable_memory_mb);
  for(size_t i = 0; i < mem->usable_range_count; i++){
    Memory_Range *range = &mem->usable_range[i];
    uintptr_t range_end = range->address + range->size;
    uint64_t range_size_mb = range->size / (1024*1024);
    klog_debug("  0x%X - 0x%X: %luMB", range->address, range_end, range_size_mb);
  }
}

static inline
void kmem_clear_page(void *page){
  //TODO(Torin) Make this a global function pointer
  //And create SSE and AVX variants
  memset(page, 0x00, 4096);
}

static inline
void kmem_set_page_state(Kernel_Memory_State *state, uint64_t index, bool used){
  uint64_t bitmap_index = index / 64; 
  uint64_t bit_index = index % 64;
  state->page_usage_bitmap[bitmap_index] |= used << bit_index;
}

static inline
void kmem_add_usable_range(uintptr_t address, uint64_t size, Kernel_Memory_State *mem){
  if(mem->usable_range_count + 1 > ARRAY_COUNT(mem->usable_range)){
    uintptr_t range_end = address + size;
    klog_warning("maximum memory range entries was execeded.  Memory_Range 0x%X - 0x%X is unusable", address, range_end);
    return;
  }

  mem->usable_range[mem->usable_range_count].address = address;
  mem->usable_range[mem->usable_range_count].size = size;
  mem->usable_range_count++;
  mem->total_usable_memory += size;
}

static inline
bool kmem_usable_range_contains(Kernel_Memory_State *memstate, uintptr_t address, size_t size){
  uintptr_t target_end = address + size;
  for(size_t i = 0; i < memstate->usable_range_count; i++){
    Memory_Range *range = &memstate->usable_range[i];
    uintptr_t range_end = range->address + range->size;
    if((address >= range->address) && (target_end <= range_end)) return true;
  }
  return false;
}

bool kmem_allocate_physical_pages(Kernel_Memory_State *memstate, uint64_t page_count, uintptr_t *out){
  uint64_t current_page_count = 0;
  for(size_t value_index = 0; value_index < memstate->page_usage_bitmap_size / 8; value_index++){
    if(memstate->page_usage_bitmap[value_index] == 0xFFFFFFFFFFFFFFFF) continue;
    for(size_t bit_index = 0; bit_index < 64; bit_index++){
      if((memstate->page_usage_bitmap[value_index] & (1 << bit_index)) == 0) {
        memstate->page_usage_bitmap[value_index] |= (1 << bit_index);
        out[current_page_count] = memstate->kernel_memory_start_virtual_address + (((value_index * 64) + bit_index) * 4096);
        current_page_count++;
        if(current_page_count >= page_count) return true;
      }
    }
  }

  //NOTE(Torin 2016-10-20) We failed to allocated enough physical pages
  //We re-iterrate and unmark the pages we just flaged as used and defer to 
  //disk swaping mechanisim inorder to meet the memory requirements
  klog_error("OUT OF MEMORY");
  for(size_t i = 0; i < current_page_count; i++){
    uint64_t page_index = (*out - memstate->kernel_memory_start_virtual_address) / 4096;
    uint64_t value_index = page_index / 64;
    uint64_t bit_index = page_index % 64; 
    memstate->page_usage_bitmap[value_index] &= (~(1 << bit_index));
  }

  //TODO(Torin 2016-10-20) Recover from running out of physical frames
  kpanic();
  return false;
}

uintptr_t kmem_allocate_persistant_kernel_memory(Kernel_Memory_State *memstate, uint64_t page_count){
  kassert(page_count < 512); //TODO(Torin 2016-10-20) This is garbage how should this be handled!
  //Mabye the procedure to obtain physical pages should only search a single page?
  //Or just define that the maximum number of pages that can be searched is 512?
  //Or another value that works well with SIMD optimization?
  uintptr_t physical_page_addressess[512] = {};
  kmem_allocate_physical_pages(memstate, page_count , physical_page_addressess);
  if((memstate->kernel_memory_used_page_count % 512 + page_count) > 512){
    //TODO(Torin 2016-10-20) We need to allocate another Page_Table so we can map another 2MB
    klog_error("reached unimplemented feature");
    kpanic();
  }

  uint64_t p2_table_offset = memstate->kernel_memory_start_virtual_address / 0x200000;
  uint64_t p2_table_index = (memstate->kernel_memory_used_page_count / 512) + p2_table_offset;
  uint64_t p1_table_index = memstate->kernel_memory_used_page_count % 512;
  Page_Table *pt = (Page_Table *)((uintptr_t)g_p2_table.entries[p2_table_index] & ~0xFFF);
  for(size_t page_index = 0; page_index < page_count; page_index++){
    pt->entries[p1_table_index + page_index] = physical_page_addressess[page_index] | PAGE_PRESENT_BIT | PAGE_WRITEABLE_BIT;
  }

  uintptr_t virtual_address = memstate->kernel_memory_start_virtual_address + (memstate->kernel_memory_used_page_count*4096);
  memstate->kernel_memory_used_page_count += page_count;
  klog_debug("[KMEM] Allocated %lu pages of persistant memory mapped to 0x%X", page_count, virtual_address);
  return virtual_address;
}

static inline
void kmem_flush_tlb(){
  asm volatile(".intel_syntax noprefix");
  asm volatile("mov rax, cr3");
  asm volatile("mov cr3, rax");
  asm volatile(".att_syntax prefix");
}

void kmem_initalize_memory_state(Kernel_Memory_State *memstate){
  extern uint32_t _KERNEL_END;
  uintptr_t kernel_end = 0x100000 + _KERNEL_END;
  uintptr_t memory_begin = (kernel_end + 0x200000) & ~0x1FFFFF;
  klog_debug("kernel_end: 0x%X", kernel_end);
  klog_debug("memory_begin: 0x%X", memory_begin);
  memstate->allocator_start = memory_begin; 
  //NOTE(Torin) The kernel is currently identity mapped!
  memstate->kernel_memory_start_virtual_address = memstate->allocator_start;
  //NOTE(Torin) This is our simple hacky way of memory managment until
  //I come up with somthing better.
  //Get Page aligned memory size
  uint64_t aligned_memory_size = (memstate->total_usable_memory + 0xFFF) & ~0xFFF;
  uint64_t mappable_page_count = aligned_memory_size / 0x1000;
  memstate->page_usage_bitmap_size = ((mappable_page_count + 0x3F) & ~0x3F) / 0x40;
  memstate->page_usage_bitmap_page_count = ((mappable_page_count + 0x7FFF) & ~0x7FFF) / 0x8000;
  //NOTE(Torin) For now the kernel begins setting up the memory managment
  //data structures at a 2MB aligned address after the end of the kernel
  //These structures are currently identity mapped which will probably be 
  //problematic in the future
  g_p2_table.entries[1] = (uintptr_t)memstate->allocator_start | PAGE_PRESENT_BIT | PAGE_WRITEABLE_BIT | PAGE_HUGE_BIT;
  kmem_flush_tlb();
  Page_Table *pt = (Page_Table *)memstate->allocator_start;
  pt->entries[0] = (uintptr_t)pt | PAGE_PRESENT_BIT | PAGE_WRITEABLE_BIT;
  memstate->page_usage_bitmap = (uint64_t *)(memstate->allocator_start + 4096);
  for(size_t i = 0; i < memstate->page_usage_bitmap_page_count; i++)
    kmem_clear_page((void *)((uintptr_t)memstate->page_usage_bitmap + (i * 4096)));
  for(size_t i = 0; i < memstate->page_usage_bitmap_page_count; i++)
    pt->entries[i+1] = ((uintptr_t)memstate->page_usage_bitmap + (i * 4096)) | PAGE_PRESENT_BIT | PAGE_WRITEABLE_BIT;
  kmem_set_page_state(memstate, 0, 1);
  for(size_t i = 0; i < memstate->page_usage_bitmap_page_count; i++) 
    kmem_set_page_state(memstate, i+1, 1);
  g_p2_table.entries[1] = (uintptr_t)pt | PAGE_PRESENT_BIT | PAGE_WRITEABLE_BIT; 
  memstate->kernel_memory_used_page_count = memstate->page_usage_bitmap_page_count + 1;
  memstate->kernel_memory_allocated_page_table_count = 1;
  kmem_flush_tlb();
}

uintptr_t kmem_map_physical_mmio(Kernel_Memory_State *memstate, uintptr_t physical_address, uint64_t page_count){
  kassert(page_count <= 512);
  kassert((physical_address & 0xFFF) == 0);
  if((memstate->kernel_memory_used_page_count + page_count) / 512 > memstate->kernel_memory_allocated_page_table_count){
    //TODO(Torin 2016-10-20) Allocate a new page table to continue allocating kernel virtual memory addresses
    //NOTE(Torin 2016-10-20) UNIMPLEMENTED FEATURE WILL CURRENTLY BREAK TASKING SYSTEM WHICH EXPECTS EXECUTABLES AT 0x400000
    kassert(false);
    kpanic();
  }

  if(kmem_usable_range_contains(memstate, physical_address, page_count*4096)){
    klog_error("cannot map physical address for MMIO because it is contained within the usable range of memory");
    kpanic();
  }

  Page_Table *pt = (Page_Table *)memstate->kernel_memory_start_virtual_address; 
  for(size_t i = 0; i < page_count; i++){
    pt->entries[memstate->kernel_memory_used_page_count + i] = (physical_address + (4096*i)) | PAGE_PRESENT_BIT | PAGE_WRITEABLE_BIT;
  }

  uintptr_t mapped_virtual_address = memstate->kernel_memory_start_virtual_address;
  mapped_virtual_address += (memstate->kernel_memory_used_page_count * 4096);
  memstate->kernel_memory_used_page_count += page_count;
  klog_debug("[KMEM] Mapped Physical: 0x%X to Virtual: 0x%X", physical_address, mapped_virtual_address);
  return mapped_virtual_address;
}

uintptr_t kmem_get_physical_address(uintptr_t virtual_address){
  uintptr_t p4_index = (virtual_address >> 39) & 0x1FF;
  uintptr_t p3_index = (virtual_address >> 30) & 0x1FF;
  uintptr_t p2_index = (virtual_address >> 21) & 0x1FF;
  uintptr_t p1_index = (virtual_address >> 12) & 0x1FF;
  Page_Table *p4_table = (Page_Table *)&g_p4_table;
  Page_Table *p3_table = (Page_Table *)(p4_table->entries[p4_index] & ~0xFFF);
  Page_Table *p2_table = (Page_Table *)(p3_table->entries[p3_index] & ~0xFFF);
  Page_Table *p1_table = (Page_Table *)(p2_table->entries[p2_index] & ~0xFFF);
  uintptr_t physical_address = p1_table->entries[p1_index] & ~0xFFF;
  return physical_address;
}

//TODO(Torin 2016-10-20) This is an astonishingly terrible idea!  It *total wont* come back to bite us in the ass later.
//GET RID OF THIS AND MAKE A PROPER TEMPORARY PAGE ALLOCATION MECHANISIM!
void kmem_map_physical_to_virtual_unaccounted(Kernel_Memory_State *memstate, uintptr_t physical_address, uintptr_t virtual_address, uint64_t flags){
  kassert((virtual_address & 0x1FFFFF) == 0);
  kassert((physical_address & 0x1FFFFF) == 0);
  if(kmem_usable_range_contains(memstate, physical_address, 0x200000)){
    klog_error("[KMEM] Attempted to map physical address contained within the usable range");
    return;
  }

  uintptr_t p4_index = (virtual_address >> 39) & 0x1FF;
  uintptr_t p3_index = (virtual_address >> 30) & 0x1FF;
  uintptr_t p2_index = (virtual_address >> 21) & 0x1FF;
  Page_Table *p4_table = (Page_Table *)&g_p4_table;
  Page_Table *p3_table = (Page_Table *)(p4_table->entries[p4_index] & ~0xFFF); 
  Page_Table *p2_table = (Page_Table *)(p3_table->entries[p3_index] & ~0xFFF);
  g_p2_table.entries[p2_index] = physical_address | PAGE_PRESENT_BIT | PAGE_HUGE_BIT | PAGE_WRITEABLE_BIT | flags;
}

uintptr_t kmem_map_unaligned_physical_to_aligned_virtual_unaccounted(Kernel_Memory_State *memstate, uintptr_t requested_physical_address, uintptr_t virtual_address, uint64_t flags){
  uint64_t physical_address_to_map = requested_physical_address;
	uint64_t displacement_from_page_boundray = requested_physical_address & 0x1FFFFF;
  physical_address_to_map -= displacement_from_page_boundray;
  kmem_map_physical_to_virtual_unaccounted(memstate, physical_address_to_map, virtual_address, flags);
  return displacement_from_page_boundray;
}