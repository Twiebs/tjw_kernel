
void ext2fs_debug_log_directory_entry(Ext2_Filesystem *fs, Ext2_Directory_Entry *directory_entry);
void ext2fs_debug_log_inode(Ext2_Inode *inode);
void ext2fs_debug_log_fs_info(Ext2_Filesystem *extfs);

void ext2fs_debug_log_directory_entry(Ext2_Filesystem *fs, Ext2_Directory_Entry *directory_entry){
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

void ext2fs_debug_log_inode(Ext2_Inode *inode){
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

void ext2fs_debug_log_group_descriptor(Ext2_Block_Group_Descriptor *descriptor){
  klog_debug("block_usage_bitmap_block_number: %u", descriptor->block_usage_bitmap_block_number);
  klog_debug("inode_usage_bitmap_block_number: %u", descriptor->inode_usage_bitmap_block_number);
  klog_debug("inode_table_block_number: %u", descriptor->inode_table_block_number);
  //uint16_t unallocated_block_count;
  //uint16_t unallocated_inode_count;
  klog_debug("directory_count: %u", (uint32_t)descriptor->directory_count); 
}

void ext2fs_debug_log_fs_info(Ext2_Filesystem *extfs){
  klog_debug("extfs_info:");
  klog_debug(" inode_count_per_goup: %u", extfs->inode_count_per_group);
  klog_debug(" inode_size: %u", extfs->inode_size);
  klog_debug(" block_size: %u", extfs->block_size);
  klog_debug(" sectors_per_block: %u", extfs->sectors_per_block);
  //klog_debug(" buffer_physical_address: 0x%X", (uint64_t)extfs->buffer_physical_address);
}