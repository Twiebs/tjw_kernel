
typedef struct {
  size_t current_used;
  uint8_t *current_virtual_block;
} Persistent_Block_Allocator;

Error_Code persistent_block_allocator_get_new_block(Persistent_Block_Allocator *allocator);
uint8_t *persistent_block_allocator_allocate(Persistent_Block_Allocator *allocator, size_t size, size_t alignment);