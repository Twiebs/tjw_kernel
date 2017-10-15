
Error_Code persistent_block_allocator_get_new_block(Persistent_Block_Allocator *allocator) {
  allocator->current_virtual_block = memory_allocate_persistent_virtual_pages(1);
  if (allocator->current_virtual_block == NULL) {
    klog_error("Persistant_Block_Allocator can not acquire system memory");
    return Error_Code_OUT_OF_MEMORY;
  }

  allocator->current_used = 0;
  return Error_Code_NONE;
}

uint8_t *persistent_block_allocator_allocate(Persistent_Block_Allocator *allocator, size_t size, size_t alignment) {
  if (allocator->current_virtual_block == NULL) {
  	allocator->current_used = 0;
  	allocator->current_virtual_block = memory_allocate_persistent_virtual_pages(1);
  	if (allocator->current_virtual_block == NULL) {
  	  klog_error("Persistant_Block_Allocator can not acquire system memory");
  	  return NULL;
  	}
  }

  size_t required_alignment_padding = alignment - (allocator->current_used & (alignment - 1));
  size_t required_memory =  required_alignment_padding + size;
  if (required_memory > 4096) { return NULL; }
  if (allocator->current_used + required_memory > 4096) {
  	persistent_block_allocator_get_new_block(allocator);
  }

  uint8_t *result = allocator->current_virtual_block + memory_align(allocator->current_used, alignment);
  allocator->current_used += required_memory;
  return result;
}