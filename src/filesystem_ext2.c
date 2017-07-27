



static const char *DIRECTORY_ENTRY_TYPE_NAMES[] = {
  "UNKNOWN_TYPE",     //0
  "REGULAR_FILE",     //1
  "DIRECTORY",        //2
  "CHARACTER_DEVICE", //3
  "BLOCK_DEVICE",     //4
  "FIFO",             //5
  "SCOKET",           //6
  "SYMBOLIC_LINK"     //7
};

//===================================================================================================

void ext2fs_log_directory_entry(Ext2_Filesystem *fs, Ext2_Directory_Entry *directory_entry){
  klog_debug("directory_entry");
  klog_debug("  inode: %u", directory_entry->inode);
  klog_debug("  size: %u", (uint32_t)directory_entry->entry_size);
  uint32_t name_length = 0;
  if(fs->required_features & EXT2_REQUIRED_FEATURE_DIRECTORY_ENTRYIES_CONTAIN_TYPE){
    if(directory_entry->type > 7) klog_debug("  directory_entry has invalid type: %u", (uint32_t)directory_entry->type);
    else klog_debug("  type: %s", DIRECTORY_ENTRY_TYPE_NAMES[directory_entry->type]);
    name_length = directory_entry->name_length;
  } else {
    klog_debug("  type: FEATURE UNUSED");
    name_length = directory_entry->name_length;
    name_length |= directory_entry->type << 8; 
  }

  klog_debug("  name_length: %u", name_length);
  klog_debug("  name: %.*s", name_length, directory_entry->name);
}

void ext2fs_log_inode(Ext2_Inode *inode){
  uint8_t type = inode->type;
  const char *type_name = "unknown";
  if(type == EXT2_INODE_TYPE_FIFO) type_name = "FIFO";
  else if(type == EXT2_INODE_TYPE_CHARACTER_DEVICE) type_name = "character_device";
  else if(type == EXT2_INODE_TYPE_DIRECTORY) type_name = "directory";
  else if(type == EXT2_INODE_TYPE_BLOCK_DEVICE) type_name = "block_device";
  else if(type == EXT2_INODE_TYPE_REGULAR_FILE) type_name = "regular_file";
  else if(type == EXT2_INODE_TYPE_SYMBOLIC_LINK) type_name = "symbolic_link";
  else if(type == EXT2_INODE_TYPE_UNIX_SOCKET) type_name = "unix_socket";

  uint64_t file_size = (uint64_t)inode->size_low; 
  file_size |= ((uint64_t)inode->size_high << 32LL);
  klog_debug(" type: %u (%s)", (uint32_t)type, type_name);
  klog_debug(" file_size: %lu", file_size);
  klog_debug(" last_access_time: %u", inode->last_access_time);
  klog_debug(" creation_time: %u", inode->creation_time);
  klog_debug(" last_modification_time: %u", inode->last_modification_time);
  klog_debug(" direct_block_pointer_0: %u", inode->direct_block_pointers[0]);
}

void ext2fs_log_group_descriptor(Ext2_Block_Group_Descriptor *descriptor){
  klog_debug("block_usage_bitmap_block_number: %u", descriptor->block_usage_bitmap_block_number);
  klog_debug("inode_usage_bitmap_block_number: %u", descriptor->inode_usage_bitmap_block_number);
  klog_debug("inode_table_block_number: %u", descriptor->inode_table_block_number);
  //uint16_t unallocated_block_count;
  //uint16_t unallocated_inode_count;
  klog_debug("directory_count: %u", (uint32_t)descriptor->directory_count); 
}

void ext2fs_log_fs_info(Ext2_Filesystem *extfs){
  klog_debug("extfs_info:");
  klog_debug(" inode_count_per_goup: %u", extfs->inode_count_per_group);
  klog_debug(" inode_size: %u", extfs->inode_size);
  klog_debug(" block_size: %u", extfs->block_size);
  klog_debug(" sectors_per_block: %u", extfs->sectors_per_block);
  //klog_debug(" buffer_physical_address: 0x%X", (uint64_t)extfs->buffer_physical_address);
}

static inline
uint32_t ext2fs_get_sector_location(uint32_t block_number, Ext2_Filesystem *extfs){
  uint32_t result = (block_number * extfs->sectors_per_block) + extfs->partition_first_sector;
  return result;
}

bool ext2fs_read_block(Ext2_Filesystem *extfs, uint32_t block_number, uintptr_t buffer_physical){
  uint32_t physical_storage_sector = (extfs->sectors_per_block * block_number) + extfs->partition_first_sector;
  if(storage_device_read(&extfs->storage_device, physical_storage_sector, 2, buffer_physical) == 0){
    klog_error("failed to read from storage device");
    return false; 
  }
  return true;
}

int ext2fs_read_inode(Ext2_Filesystem *extfs, uint32_t inode_number, Ext2_Inode *out_inode){
  strict_assert(extfs->inode_count_per_group > 0);
  strict_assert(extfs->block_size > 0);
  strict_assert(extfs->inode_size > 0);
  klog_debug("attempting to read ext2fs inode: %u", inode_number);

  uint32_t descriptors_per_block = extfs->block_size / 32;
  uint32_t group_index = (inode_number - 1) / extfs->inode_count_per_group;
  uint32_t group_block = (group_index * 32) / extfs->block_size;
  uint32_t group_descriptor_index = group_index % descriptors_per_block;

  uint32_t inode_index = (inode_number - 1) % extfs->inode_count_per_group;
  uint32_t inode_table_block_index = (inode_index * extfs->inode_size) / extfs->block_size;
  uint32_t inode_block_offset = inode_index - (inode_table_block_index *(extfs->inode_size / extfs->block_size));

#if 0
  klog_debug("group_index: %u", group_index);
  klog_debug("group_block: %u", group_block);
  klog_debug("group_descriptor_index: %u", group_descriptor_index);
  klog_debug("inode_index: %u", inode_index);
  klog_debug("inode_table_block_index: %u", inode_table_block_index);
  klog_debug("inode_block_offset: %u", inode_block_offset);
#endif

  uint8_t buffer[4096] = {};
  if(ext2fs_read_block(extfs, group_block + 1, (uintptr_t)buffer) == 0){
    klog_error("failed to read group descriptor table");
    return 0;
  }
   
  Ext2_Block_Group_Descriptor *group_descriptor_table = (Ext2_Block_Group_Descriptor *)buffer; 
  Ext2_Block_Group_Descriptor *group_descriptor = &group_descriptor_table[group_descriptor_index];
  
  if(ext2fs_read_block(extfs, group_descriptor->inode_table_block_number + inode_table_block_index, (uintptr_t)buffer) == 0){
    klog_error("failed to read inode table");
    return 0;
  }

  Ext2_Inode *inode = (Ext2_Inode *)(buffer + (inode_block_offset * extfs->inode_size)); 
  *out_inode = *inode;
  return 1;
}