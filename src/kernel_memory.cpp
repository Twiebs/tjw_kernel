
struct PageMapLevel4Table {
	uint64_t entries[512];
};

struct PageDirectoryPointerTable {
	uint64_t entries[512];
};

struct PageDirectoryTable {
  uint64_t entries[512];
};

struct PageTable {
  uint64_t entries[512];
};

global_variable PageMapLevel4Table _p4 __attribute__((aligned(4096)));
global_variable PageDirectoryPointerTable _p3 __attribute((aligned(4096)));
global_variable PageDirectoryTable _p2 __attribute((aligned(4096)));
global_variable PageTable _page_tables[16] __attribute((aligned(4096)));

internal void
x64_initalize_paging() 
{
	static const uint64_t PRESENT_BIT 	= 1 << 0;
	static const uint64_t WRITEABLE_BIT = 1 << 1;
	static const uint64_t HUGE_PAGE_BIT = 1 << 7;

	_p4.entries[0] = (uint64_t)(&_p3);
	_p4.entries[0] |= (PRESENT_BIT | WRITEABLE_BIT); 
	_p3.entries[0] = (uint64_t)(&_p2);
	_p3.entries[0] |= (PRESENT_BIT | WRITEABLE_BIT);

	for (uint64_t i = 0; i < 16; i++) 
	{
		uint64_t physical_address = 0x200000 * i;
		_page_tables[i] = physical_address;
		_page_tables[i] |= (PRESENT_BIT | WRITEABLE_BIT | HUGE_PAGE_BIT);
	}
}

#if 0

struct PageInfo {
	uint32_t present : 1;
	uint32_t read_write : 1;
	uint32_t user_mode: 1;
	uint32_t is_accessed : 1;
	uint32_t is_dirty : 1;
	uint32_t _UNUSED : 7;
	uint32_t frame_addr : 20;
};

struct PageTable {
	uint32_t page_table_entries[1024];
};

struct PageDirectory {
	uint32_t page_directory_entries[1024];
};

#if 1
global_variable PageDirectory _page_directory __attribute__((aligned(4096)));
global_variable PageTable _first_page_table __attribute__((aligned(4096)));
#endif

external void asm_load_page_directory_and_enable_paging(uint32_t *pagedir);
#if 1
internal void
kmem_initialize() {
	const uint32_t total_memory_size = 1024*1024*16;
	const uint32_t page_count = total_memory_size / 4096;

	memset(&_page_directory, 0, sizeof(PageDirectory));
	_page_directory.page_directory_entries[0] = ((uintptr_t)(_first_page_table.page_table_entries)) | 3;
	for (uint32_t i = 0; i < 1024; i++) {
		_first_page_table.page_table_entries[i] = (i * 0x1000) | 0b00; 
	}

	asm_load_page_directory_and_enable_paging((uint32_t *)(_page_directory.page_directory_entries));
	klog("paging enabled");
}
#endif

#if 0
struct PageDirectory {
	PageTable *tables[1024];
	uint32_t table_physical_addr[1024];
	uint32_t physical_addr;
};

struct MemoryState {
	uint32_t placement_address;
	uint32_t active_frame_count;
	uint32_t *frame_state_bitsets;
	PageDirectory *page_directory;
	PageDirectory *current_directory;
};

global_variable MemoryState __kmemstate;
internal inline MemoryState *___get_memstate() {
	return &__kmemstate;
}

#define get_memstate ___get_memstate

internal uintptr_t 
kmem_allocate(size_t size, uint32_t alignment) {
	MemoryState *memstate = get_memstate();
	uintptr_t alignment_mask = alignment - 1;
	if (memstate->placement_address & alignment_mask) {
		uintptr_t alignment_offset = alignment - (memstate->placement_address & alignment_mask);
		memstate->placement_address += alignment_offset;
	}

	uintptr_t result = _kmemstate.placement_address;
	_kmemstate.placement_address += size;
	return result;
}

internal uintptr_t 
kmem_find_first_available_frame() {
	MemoryState *memstate = get_memstate();
	uint32_t frame_state_bitset_array_count = _frame_count / 32;
	for (uint32_t i = 0; i < frame_state_bitset_array_count; i++) {
		if (memstate->frames[i] != 0xFFFFFFFF) {
			for (uint32_t j = 0; j < 32; j++) {
				uint32_t mask = 0x1 << j;
				if (!(memstate->frames[i] & mask)) {
					return i * 4 * 8 + j
				}
			}
		}
	}
}

internal PageInfo 
kmem_frame_allocate() {
	

}

internal void 
kmem_frame_release(PageInfo *info) {

}

internal void
kmem_initalize() {
	MemoryState *memstate = get_memstate();

	//TODO(Torin) This is absurd  
	uintptr_t mem_end_page = 0x1000000;
	memstate->frame_count = mem_end_page / 0x1000;
	memstate->frame_state_bitsets = 
		(uint32_t *)kmem_allocate(memstate->frame_count / 32);
	memset(memstate->frame_state_bitsets, 0, memstate->frame_count / 32);

	//TODO(Torin) This is even worse
	memstate->page_directory = (PageDirectory *)kmem_allocate(sizeof(PageDirectory));
	memset(memestate->page_directory, 0, sizeof(PageDirectory));
	memstate->current_directory = memstate->page_directory;

	uint32_t i = 0;
	while (i < placement_address) {
		kmem_frame_
	}


}
#endif

#undef get_memstate
#endif
