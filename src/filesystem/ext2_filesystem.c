
#include "ext2_debug.c"

Error_Code ext2_block_read(Ext2_Filesystem *ext2fs, uint32_t block_number, uint8_t *buffer) {
  uint32_t physical_storage_sector = (ext2fs->sectors_per_block * block_number) + ext2fs->partition_first_sector;
  if (storage_device_read(ext2fs->storage_device, physical_storage_sector, ext2fs->sectors_per_block, buffer)) {
    klog_error("failed to read from storage device");
    return Error_Code_FAILED_READ; 
  }
  return Error_Code_NONE;
}

Error_Code ext2_inode_read(Ext2_Filesystem *extfs, uint32_t inode_number, Ext2_Inode *out_inode) {
  strict_assert(extfs->inode_count_per_group > 0);
  strict_assert(extfs->block_size > 0);
  strict_assert(extfs->inode_size > 0);

  uint8_t buffer[4096] = {};

  uint32_t descriptors_per_block = extfs->block_size / sizeof(Ext2_Block_Group_Descriptor);
  uint32_t group_descriptor_index = (inode_number - 1) / extfs->inode_count_per_group;
  uint32_t inode_table_index = (inode_number - 1) % extfs->inode_count_per_group;
  uint32_t group_block_number = group_descriptor_index / descriptors_per_block;
  uint32_t group_block_index = group_descriptor_index % descriptors_per_block;

  //TODO(Torin: 2017-08-06) Change plus 1 to actual offset to block group descriptor table
  if (ext2_block_read(extfs, group_block_number + 1, buffer)) {
    klog_error("failed to read group descriptor table");
    return Error_Code_FAILED_READ;
  }

  Ext2_Block_Group_Descriptor *group_descriptor = (Ext2_Block_Group_Descriptor *)&buffer[group_block_index*sizeof(Ext2_Block_Group_Descriptor)];
  uint32_t inode_table_start_block = group_descriptor->inode_table_block_number;

  uint32_t inode_table_block_number = (inode_table_index * extfs->inode_size) / extfs->block_size;
  uint32_t inode_table_block_offset = (inode_table_index * extfs->inode_size) % extfs->block_size;
  if (ext2_block_read(extfs, inode_table_start_block + inode_table_block_number, buffer)) {
    klog_error("failed to read inode table");
    return Error_Code_FAILED_READ;
  }

  Ext2_Inode *inode = (Ext2_Inode *)(buffer + inode_table_block_offset);
  *out_inode = *inode;
  return Error_Code_NONE;
}

//TODO(Torin: 2017-08-10) This is slooooooowwwww.  Use some sort of streamed reader?  Will
//wait until filesystem caching is impleented
Error_Code ext2_inode_get_data_block_number(Ext2_Filesystem *ext2fs, Ext2_Inode *inode, uint32_t block_index, uint32_t *result) {
  uint32_t EXT2_INODE_DIRECT_BLOCK_COUNT = 12;
  uint32_t block_numbers_per_block = ext2fs->block_size / sizeof(uint32_t);
  uint32_t max_indirect1_block_number = EXT2_INODE_DIRECT_BLOCK_COUNT + block_numbers_per_block;
  uint32_t max_indirect2_block_number = EXT2_INODE_DIRECT_BLOCK_COUNT + (block_numbers_per_block * block_numbers_per_block);

  if (block_index < EXT2_INODE_DIRECT_BLOCK_COUNT) {
    return block_index;
  } else if (block_index < max_indirect2_block_number) {
    uint8_t buffer[4096];
    if (ext2_block_read(ext2fs, inode->singly_indirect_block_pointer, buffer)) {
      return Error_Code_FAILED_READ;
    }
    uint32_t index = block_index - EXT2_INODE_DIRECT_BLOCK_COUNT;
    uint32_t *block_number_array = (uint32_t *)buffer;
    *result = block_number_array[index];
    return Error_Code_NONE;
  }


  kassert(false);
  return Error_Code_NONE;
}


Error_Code ext2_inode_read_data(Ext2_Filesystem *ext2fs, Ext2_Inode *inode, uint64_t offset, uint64_t size, uint8_t *buffer) {
  uint32_t start_block_number = offset / ext2fs->block_size;
  uint32_t blocks_to_read_count = size / ext2fs->block_size;
  if (size % ext2fs->block_size) blocks_to_read_count++;
  if (blocks_to_read_count == 0) return Error_Code_NONE;

  uint8_t temp_buffer[4096];//TODO(Torin: 2017-08-10) Were copying from a temp buffer to avoid
  //handling case when offset > 0 && size < ext2fs->block_size.  it needs to be corner cased necause
  //the buffer pointer might not have enough space
  uint32_t block_number_to_read = 0;
  uint32_t offset_into_start_block = offset % ext2fs->block_size;
  uint32_t start_block_data_size = ext2fs->block_size - offset_into_start_block;
  if (ext2_inode_get_data_block_number(ext2fs, inode, start_block_number, &block_number_to_read)) 
    return Error_Code_FAILED_READ;
  if (ext2_block_read(ext2fs, block_number_to_read, temp_buffer)) 
    return Error_Code_FAILED_READ;

  memory_copy(buffer, temp_buffer + offset_into_start_block, start_block_data_size);
  uint8_t *write_ptr = buffer + start_block_data_size;
  //NOTE(Torin: 2017-08-10) Loop only runs for blocks between start and end
  for (uint64_t i = 1; i < blocks_to_read_count - 1; i++) {
    if (ext2_inode_get_data_block_number(ext2fs, inode, start_block_number + i, &block_number_to_read)) 
      return Error_Code_FAILED_READ;
    if (ext2_block_read(ext2fs, block_number_to_read, write_ptr))
      return Error_Code_FAILED_READ;
    write_ptr += ext2fs->block_size;
  }

  //NOTE(Torin: 2017-08-10) Handle the last block case
  if (blocks_to_read_count > 1) {
    if (ext2_inode_get_data_block_number(ext2fs, inode, start_block_number + (blocks_to_read_count - 1), &block_number_to_read))
      return Error_Code_FAILED_READ;
    if (ext2_block_read(ext2fs, block_number_to_read, temp_buffer)) 
      return Error_Code_FAILED_READ;
    uint32_t size_of_last_block = (offset + size) % ext2fs->block_size;
    memory_copy(write_ptr, temp_buffer, size_of_last_block);
  }

  return Error_Code_NONE;
}


Error_Code ext2_find_inode_in_directory_from_name_string(Ext2_Filesystem *ext2fs, Ext2_Inode *directory_inode, 
const char *name, size_t name_length, uint8_t *scratch_memory, uint32_t *result_inode_number) {
  if (directory_inode->type != EXT2_INODE_TYPE_DIRECTORY) {
    klog_error("inode is not a directory");
    return Error_Code_INVALID_DATA;
  }

  if (ext2_block_read(ext2fs, directory_inode->direct_block_pointers[0], scratch_memory)) {
    klog_error("failed to read directory entries");
    return Error_Code_FAILED_READ;
  }


  Ext2_Directory_Entry *directory_entry = (Ext2_Directory_Entry *)scratch_memory;
  while (((uintptr_t)directory_entry - (uintptr_t)scratch_memory) < 4096) {
    if (directory_entry->entry_size == 0) break;
    if (string_equals_string(directory_entry->name, directory_entry->name_length, name, name_length)) {
      ext2_debug_log_directory_entry(ext2fs, directory_entry);
      //TODO(Torin) Do consitancy checks to ensure that this is a valid inode
      *result_inode_number = directory_entry->inode;
      return Error_Code_NONE;
    }
    directory_entry = (Ext2_Directory_Entry *)((uintptr_t)directory_entry + directory_entry->entry_size);
  }

  return Error_Code_VFS_NODE_NOT_FOUND;
}


Error_Code ext2_file_system_initalize(Ext2_Filesystem *extfs, Storage_Device *storage_device, uint64_t partition_index) {
  kassert(partition_index < storage_device->partition_count);
  Storage_Device_Partition *partition = &storage_device->partitions[partition_index];
  kassert(partition->file_system_type == File_System_Type_EXT2);
  
  uint32_t superblock_location = partition->first_block + 2;
  uint8_t temporary_buffer[4096]; //TODO(Torin: 2017-07-27) Get rid of this
  if (storage_device_read(storage_device, superblock_location, 1, temporary_buffer)) {
    klog_error("failed to read superblock from ext2 partition");
    return Error_Code_FAILED_READ;
  }


  Ext2_Superblock *superblock = (Ext2_Superblock *)temporary_buffer;
  if (superblock->ext2_signature != EXT2_SUPERBLOCK_SIGNATURE) {
    klog_error("superblock has invalid signature");
    return Error_Code_INVALID_DATA;
  }

  if (superblock->filesystem_state == 2) {
    klog_error("ext2fs has errors");
    //TODO(Torin: 2017-08-05) Error Handling Methods 
    return Error_Code_INVALID_DATA;
  }

  if (superblock->version_major < 1) {
    klog_error("ext2 uses version 0");
    return Error_Code_INVALID_DATA;
  }

  extfs->partition_first_sector = partition->first_block;
  extfs->inode_size = superblock->inode_struct_size;
  extfs->block_size = 1024 << superblock->block_size_shift_count;
  extfs->inode_count_per_group = superblock->inodes_per_group;
  extfs->block_count_per_group = superblock->blocks_per_group;
  extfs->superblock_sector = superblock_location;
  extfs->required_features = superblock->required_features;
  extfs->storage_device = storage_device;

  strict_assert((uintptr_t)temporary_buffer + 4096 < 0x200000);
  if (extfs->required_features & EXT2_REQUIRED_FEATURE_COMPRESSION) {
    klog_error("FILESYSTEM USES COMPRESSION!");
    return Error_Code_UNSUPORTED_FEATURE;
  }
  if (extfs->required_features & EXT2_REQUIRED_FEATURE_JOURNAL_DEVICE) {
    klog_warning("FILESYSTEM USES A JOURNAL DEVICE");
    return Error_Code_UNSUPORTED_FEATURE;
  }
  if (extfs->required_features & EXT2_REQUIRED_FEATURE_REPLAY_JOURNAL) {
    klog_warning("FILESYSTEM MUST REPLAY JOURNAL");
    return Error_Code_UNSUPORTED_FEATURE;
  }

  uint32_t group_count_calculated_from_blocks = superblock->block_count / superblock->blocks_per_group;
  uint32_t group_count_calculated_from_inodes = superblock->inode_count / superblock->inodes_per_group;
  if (superblock->block_count % superblock->blocks_per_group != 0) 
    group_count_calculated_from_blocks++;
  if (superblock->inode_count % superblock->inodes_per_group != 0) 
    group_count_calculated_from_inodes++;
  if (group_count_calculated_from_blocks != group_count_calculated_from_inodes) {
    klog_error("INVALID EXT2 GROUP COUNT!");
    return Error_Code_INVALID_DATA;
  }

  extfs->group_count = group_count_calculated_from_blocks;
  if (extfs->block_size % storage_device->block_size != 0) { 
    klog_error("filesystem block size not multiple of storage_device block size");
    return Error_Code_INVALID_DATA;
  }

  extfs->sectors_per_block = extfs->block_size / storage_device->block_size;
  return Error_Code_NONE;
}