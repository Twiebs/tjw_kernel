
#include "ext2fs_debug.c"

uint32_t ext2fs_get_sector_location(uint32_t block_number, Ext2_Filesystem *extfs) {
  uint32_t result = (block_number * extfs->sectors_per_block) + extfs->partition_first_sector;
  return result;
}

bool ext2fs_read_block(Ext2_Filesystem *extfs, uint32_t block_number, uintptr_t buffer_physical){
  uint32_t physical_storage_sector = (extfs->sectors_per_block * block_number) + extfs->partition_first_sector;
  if (storage_device_read_to_physical(extfs->storage_device, physical_storage_sector, 2, buffer_physical) == 0){
    klog_error("failed to read from storage device");
    return false; 
  }
  return true;
}

int ext2fs_read_inode(Ext2_Filesystem *extfs, uint32_t inode_number, Ext2_Inode *out_inode) {
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


int ext2_file_system_initalize(Ext2_Filesystem *extfs, Storage_Device *storage_device, uint64_t partition_index) {
  kassert(partition_index < storage_device->partition_count);
  Storage_Device_Partition *partition = &storage_device->partitions[partition_index];
  kassert(partition->file_system_type == File_System_Type_EXT2);
  
  uint32_t superblock_location = partition->first_block + 2;
  uint8_t temporary_buffer[4096]; //TODO(Torin: 2017-07-27) Get rid of this
  if (storage_device_read_to_physical(storage_device, superblock_location, 1, (uintptr_t)temporary_buffer)) {
    klog_error("failed to read superblock from ext2 partition");
    return -1;
  }


  Ext2_Superblock *superblock = (Ext2_Superblock *)temporary_buffer;
  if (superblock->ext2_signature != EXT2_SUPERBLOCK_SIGNATURE) {
    klog_error("superblock has invalid signature");
    return -2;
  }

  if (superblock->version_major < 1) {
    klog_error("ext2 uses version 0");
    return -2;
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
  if (extfs->required_features & EXT2_REQUIRED_FEATURE_COMPRESSION)
    klog_warning("FILESYSTEM USES COMPRESSION!");
  if (extfs->required_features & EXT2_REQUIRED_FEATURE_JOURNAL_DEVICE)
    klog_warning("FILESYSTEM USES A JOURNAL DEVICE");
  if (extfs->required_features & EXT2_REQUIRED_FEATURE_REPLAY_JOURNAL)
    klog_warning("FILESYSTEM MUST REPLAY JOURNAL");

  uint32_t group_count_calculated_from_blocks = superblock->block_count / superblock->blocks_per_group;
  uint32_t group_count_calculated_from_inodes = superblock->inode_count / superblock->inodes_per_group;
  if (superblock->block_count % superblock->blocks_per_group != 0) 
    group_count_calculated_from_blocks++;
  if (superblock->inode_count % superblock->inodes_per_group != 0) 
    group_count_calculated_from_inodes++;
  if (group_count_calculated_from_blocks != group_count_calculated_from_inodes) {
    klog_error("INVALID EXT2 GROUP COUNT!");
    return -2;
  }

  extfs->group_count = group_count_calculated_from_blocks;
  if (extfs->block_size % storage_device->block_size != 0) { 
    klog_error("filesystem block size not multiple of storage_device block size"); 
  }

  extfs->sectors_per_block = extfs->block_size / storage_device->block_size;
  return 0;
}





#if 0
    klog_debug("inode_count: %u", (uint32_t)superblock->inode_count);
    klog_debug("block_count: %u", (uint32_t)superblock->block_count);
    klog_debug("group_count: %u", (uint32_t)extfs->group_count);
    
    uint32_t current_block_count = superblock->block_count - superblock->unallocated_blocks;
    uint32_t current_inode_count = superblock->inode_count - superblock->unallocated_inodes;
    klog_debug("unallocated_inodes: %u", superblock->unallocated_inodes);
    klog_debug("unallocated_blocks: %u", superblock->unallocated_blocks);
    klog_debug("allocated_blocks: %u", current_block_count);
    klog_debug("allocated_inodes: %u", current_inode_count);

    uint32_t block_group_descriptor_table_sector = ext2fs_get_sector_location(1, extfs); 
    klog_debug("bock_group_descriptor_table_sector: %u", block_group_descriptor_table_sector);
    if(ehci_read_to_physical_address(hc, msd, buffer_physical_address, block_group_descriptor_table_sector, 1) == 0){
      klog_error("failed to read block_group_descriptor_table");
      return 0;
    }

    Ext2_Block_Group_Descriptor *block_group_descriptor = (Ext2_Block_Group_Descriptor *)read_buffer;
    klog_debug("directory_count: %u", (uint32_t)block_group_descriptor->directory_count);

    uint32_t allocated_block_count = extfs->block_count_per_group - block_group_descriptor->unallocated_block_count;
    uint32_t allocated_inode_count = extfs->inode_count_per_group - block_group_descriptor->unallocated_inode_count;
    klog_debug("unallocated_block_count: %u", (uint32_t)block_group_descriptor->unallocated_block_count);
    klog_debug("unallocated_inode_count: %u", (uint32_t)block_group_descriptor->unallocated_inode_count);
    klog_debug("allocated_block_count: %u", allocated_block_count);
    klog_debug("allocated_inode_count: %u", allocated_inode_count);

    uint32_t inode_table_sector = ext2fs_get_sector_location(block_group_descriptor->inode_table_block_number, extfs);
    klog_debug("inode_table_sector: %u", inode_table_sector);
    if(ehci_read_to_physical_address(hc, msd, buffer_physical_address, inode_table_sector, 1) == 0){
      klog_error("failed to read inode table");
      return 0;
    }

    Ext2_Inode *inode_table = (Ext2_Inode *)read_buffer;
    Ext2_Inode *root_inode = (Ext2_Inode *)(read_buffer + extfs->inode_size);
    if(root_inode->type != EXT2_INODE_TYPE_DIRECTORY){
      klog_error("root_inode is not a directory!");
      klog_debug("type: %u", (uint32_t)root_inode->type);
      return 0;
    }

    //TODO(Torin 2016-10-16) Consider renaming direct_block_pointer to direct_block_number for clarity
    uint32_t directory_entries_location = ext2fs_get_sector_location(root_inode->direct_block_pointers[0], extfs);
    if(ehci_read_to_physical_address(hc, msd, buffer_physical_address, directory_entries_location, 1) == 0){
      klog_debug("failed to read directory entriies");
      return 0;
    }

    Ext2_Directory_Entry *directory_entry = (Ext2_Directory_Entry *)read_buffer;
    while(((uintptr_t)directory_entry - (uintptr_t)read_buffer) < 4096){
      if(directory_entry->entry_size == 0) break;
      kdebug_ext2_log_directory_entry(extfs, directory_entry);
      directory_entry = (Ext2_Directory_Entry *)((uintptr_t)directory_entry + directory_entry->entry_size);
    }
#endif