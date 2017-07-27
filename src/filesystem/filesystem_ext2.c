
#include "ext2fs_debug.c"

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

