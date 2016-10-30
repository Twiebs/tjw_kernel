
typedef struct {
  uint8_t boot_indicator;
  uint8_t starting_head;
  uint8_t starting_sector : 6;
  uint8_t starting_cylinder_high : 2;
  uint8_t starting_cylinder_low;
  uint8_t system_id;
  uint8_t ending_head;
  uint8_t ending_sector : 6;
  uint8_t ending_cylinder_high : 2;
  uint8_t ending_cylinder_low;
  uint32_t start_sector;
  uint32_t sector_count;
} __attribute((packed)) MBR_Partition_Table;

typedef struct {
  uint32_t inode_count;
  uint32_t block_count;
  uint32_t reserved_blocks;
  uint32_t unallocated_blocks;
  uint32_t unallocated_inodes;
  uint32_t superblock_block_number;
  uint32_t block_size_shift_count;
  uint32_t fragment_size_shift_count;
  uint32_t blocks_per_group;
  uint32_t fragments_per_group;
  uint32_t inodes_per_group;
  uint32_t last_mount_time;
  uint32_t last_write_time;

  uint16_t mount_count_since_last_consistency_check;
  uint16_t mount_count_allowed_before_consistency_check;
  uint16_t ext2_signature; //0xEF53
  uint16_t filesystem_state; //1 = clean, 2 = errors
  uint16_t error_handling_method;
  uint16_t version_minor;

  uint32_t last_consistency_check_time;
  uint32_t forced_consistency_check_interval;
  uint32_t operating_system_id;
  uint32_t version_major;
  uint16_t user_id_of_reserved_blocks;
  uint16_t group_id_of_reserved_blocks;
  //Extended Superblock Fields(Avaiable if Version is >= 1)
  uint32_t first_non_reserved_inode;
  uint16_t inode_struct_size;
  uint16_t superblock_block_group;

  uint32_t optional_features;
  uint32_t required_features;
  uint32_t read_only_features;

  uint8_t file_system_id[16];
  uint8_t volume_name[16];
  uint8_t last_mount_path[64];

  uint32_t compression_algorithim_used;
  uint8_t blocks_count_to_preallocated_for_files;
  uint8_t block_count_to_preallocated_for_directories;
  uint16_t unused;

  uint8_t journal_id[16];
  uint32_t journal_inode;
  uint32_t journal_device;
  uint32_t head_of_orphan_inode_list;
} __attribute((packed)) Ext2_Superblock;

typedef struct {
  uint32_t block_usage_bitmap_block_number;
  uint32_t inode_usage_bitmap_block_number;
  uint32_t inode_table_block_number;
  uint16_t unallocated_block_count;
  uint16_t unallocated_inode_count;
  uint16_t directory_count;
  uint16_t padding;
  uint8_t reserved[12];
} __attribute((packed)) Ext2_Block_Group_Descriptor;

typedef struct {
  uint16_t type_and_permissions;
  uint16_t user_id;
  uint32_t size_low;
  uint32_t last_access_time;
  uint32_t creation_time;
  uint32_t last_modification_time;
  uint32_t deletion_time;
  uint16_t group_id;
  uint16_t hard_link_count; //directory_entries
  uint32_t disk_sector_count;
  uint32_t flags;
  uint32_t os_specific_value_1;
  uint32_t direct_block_pointer_0;
  uint32_t direct_block_pointer_1;
  uint32_t direct_block_pointer_2;
  uint32_t direct_block_pointer_3;
  uint32_t direct_block_pointer_4;
  uint32_t direct_block_pointer_5;
  uint32_t direct_block_pointer_6;
  uint32_t direct_block_pointer_7;
  uint32_t direct_block_pointer_8;
  uint32_t direct_block_pointer_9;
  uint32_t direct_block_pointer_10;
  uint32_t direct_block_pointer_11;
  uint32_t singly_indirect_block_pointer; //Points to a block that is a list of block pointers to data
  uint32_t doubly_indirect_block_pointer; //Points to a block that is a list of block pointers to singly_indirect_blocks
  uint32_t triply_indirct_block_pointer; //Points to a block that is a list of block pointers to doubly_indirect_blocks
  uint32_t generation_number;
  uint32_t extended_attribute_block; //Reserved in Ext2 Version 0
  uint32_t size_high; //Reserved in Ext2 Version 0
  uint32_t fragment_block_address;
  uint32_t operating_system_specific_value_2;
  uint32_t operating_system_specific_value_3;
  uint32_t operating_system_specific_value_4;
} __attribute((packed)) Ext2_Inode;

typedef struct {
  uint32_t inode;
  uint16_t entry_size;
  uint8_t name_length;
  uint8_t type;
  char name[0];
} __attribute((packed)) Ext2_Directory_Entry;

static const uint8_t PARTITION_TYPE_LINUX_FILESYSTEM = 0x83;

static const uint16_t EXT2_SUPERBLOCK_SIGNATURE = 0xEF53;
static const uint16_t EXT2_FILESYSTEM_STATE_CLEAN = 0x01;
static const uint16_t EXT2_FILESYSTEM_STATE_ERRORS = 0x02;

static const uint32_t EXT2_REQUIRED_FEATURE_COMPRESSION = 1 << 0;
static const uint32_t EXT2_REQUIRED_FEATURE_DIRECTORY_ENTRYIES_CONTAIN_TYPE = 1 << 1;
static const uint32_t EXT2_REQUIRED_FEATURE_REPLAY_JOURNAL = 1 << 2;
static const uint32_t EXT2_REQUIRED_FEATURE_JOURNAL_DEVICE = 1 << 3;

static const uint16_t EXT2_INODE_PERMISSION_OTHER_EXECUTE   = 1 << 0;
static const uint16_t EXT2_INODE_PERMISSION_OTHER_WRITE     = 1 << 1;
static const uint16_t EXT2_INODE_PERMISSION_OTHER_READ      = 1 << 2;
static const uint16_t EXT2_INODE_PERMISSION_GROUP_EXECUTE   = 1 << 3;
static const uint16_t EXT2_INODE_PERMISSION_GROUP_WRITE     = 1 << 4;
static const uint16_t EXT2_INODE_PERMISSION_GROUP_READ      = 1 << 5;
static const uint16_t EXT2_INODE_PERMISSION_USER_EXECUTE    = 1 << 6;
static const uint16_t EXT2_INODE_PERMISSION_USER_WRITE      = 1 << 7;
static const uint16_t EXT2_INODE_PERMISSION_USER_READ       = 1 << 8;
static const uint16_t EXT2_INODE_PERMISSION_STICKY_BIT      = 1 << 9;
static const uint16_t EXT2_INODE_PERMISSION_SET_GROUP_ID    = 1 << 10;
static const uint16_t EXT2_INODE_PERMISSION_SET_USER_ID     = 1 << 11;
static const uint16_t EXT2_INODE_TYPE_FIFO                  = 1 << 12; 
static const uint16_t EXT2_INODE_TYPE_CHARACTER_DEVICE      = 1 << 13; 
static const uint16_t EXT2_INODE_TYPE_DIRECTORY             = 1 << 14; 

static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_UNKNOWN_TYPE = 0;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_REGULAR_FILE = 1;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_DIRECTORY = 2;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_CHARACTER_DEVICE = 3;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_BLOCK_DEVICE = 4;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_FIFO = 5;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_SOCKET = 6;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_SYMBOLIC_LINK = 7;

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


static inline
void kdebug_ext2_log_directory_entry(Ext2_Filesystem *fs, Ext2_Directory_Entry *directory_entry){
  klog_debug("directory_entry");
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

static inline
void kdebug_ext2_log_inode(Ext2_Inode *inode){
  const char *type_name = "unknown";
  if(inode->type_and_permissions & EXT2_INODE_TYPE_DIRECTORY) type_name = "directory";
  else if(inode->type_and_permissions & EXT2_INODE_TYPE_FIFO) type_name = "FIFO";
  else if(inode->type_and_permissions & EXT2_INODE_TYPE_CHARACTER_DEVICE) type_name = "character_device";
  uint8_t type = inode->type_and_permissions >> 12;
  klog_debug(" type: %u (%s)", (uint32_t)type, type_name);
}

static inline
void ext2fs_log_group_descriptor(Ext2_Block_Group_Descriptor *descriptor){
  klog_debug("block_usage_bitmap_block_number: %u", descriptor->block_usage_bitmap_block_number);
  klog_debug("inode_usage_bitmap_block_number: %u", descriptor->inode_usage_bitmap_block_number);
  klog_debug("inode_table_block_number: %u", descriptor->inode_table_block_number);
  //uint16_t unallocated_block_count;
  //uint16_t unallocated_inode_count;
  klog_debug("directory_count: %u", (uint32_t)descriptor->directory_count); 
}

static inline
void ext2_log_fs_info(Ext2_Filesystem *extfs){
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

static int ehci_read_to_physical_address(EHCI_Controller *hc, USB_Mass_Storage_Device *msd, uintptr_t out_data, uint32_t start_block, uint16_t block_count);

//TODO(Torin 2016-10-27) Change ehci_read size to be actual byte size rather than scalar of physical block
//size because that size can vary

static inline
int storage_device_read(Storage_Device *sd, uint64_t block_index, uint64_t block_count, void *output_buffer){
  switch(sd->type){
    case Storage_Device_EHCI:{
      EHCI_Controller *ehci_hc = (EHCI_Controller *)sd->controller_ptr;
      USB_Device *device = (USB_Device *)sd->device_ptr; 
      return ehci_read_to_physical_address(ehci_hc, &device->msd, (uintptr_t)output_buffer, block_index, block_count);
    } break;

    case Storage_Device_INVALID:{
      klog_error("invalid storage device");
    } break;

    default:{
      klog_error("usupported storage device");
    } break;
  }
  
  return 0;
}

static inline
bool ext2fs_read_block(Ext2_Filesystem *extfs, uint32_t block_number, uint8_t *buffer){
  uint32_t physical_storage_sector = (extfs->sectors_per_block * block_number) + extfs->partition_first_sector;
  if(storage_device_read(&extfs->storage_device, physical_storage_sector, 1, buffer) == 0){
    klog_error("failed to read from storage device");
    return false; 
  }
  return true;
}

static inline
int ext2_read_inode(uint32_t inode_number, Ext2_Filesystem *extfs, Ext2_Inode *out_inode){
  strict_assert(extfs->inode_count_per_group > 0);
  strict_assert(extfs->block_size > 0);
  strict_assert(extfs->inode_size > 0);
  uint32_t group_index = (inode_number - 1) / extfs->inode_count_per_group;
  uint32_t group_descriptor_block_index = group_index / 32;
  uint32_t group_block_offset = group_index - (group_descriptor_block_index * 32);
  uint32_t inode_index = (inode_number - 1) % extfs->inode_count_per_group;
  uint32_t inode_table_block_index = (inode_index * extfs->inode_size) / extfs->block_size;
  uint32_t inode_block_offset = inode_index - (inode_table_block_index *(extfs->inode_size / extfs->block_size));

  klog_debug("group_index: %u", group_index);
  klog_debug("group_block_index: %u", group_descriptor_block_index);
  klog_debug("group_block_offset: %u", group_block_offset);
  klog_debug("inode_index: %u", inode_index);
  klog_debug("inode_table_block_index: %u", inode_table_block_index);
  klog_debug("inode_block_offset: %u", inode_block_offset);

  uint8_t buffer[4096] = {};
  if(ext2fs_read_block(extfs, group_descriptor_block_index + 1, buffer) == 0){
    klog_error("failed to read group descriptor table");
    return 0;
  }
   
  Ext2_Block_Group_Descriptor *group_descriptor_table = (Ext2_Block_Group_Descriptor *)buffer; 
  Ext2_Block_Group_Descriptor *group_descriptor = &group_descriptor_table[group_block_offset];
  ext2fs_log_group_descriptor(group_descriptor);

  if(ext2fs_read_block(extfs, group_descriptor->inode_table_block_number + inode_table_block_index, buffer) == 0){
    klog_error("failed to read inode table");
    return 0;
  }

  Ext2_Inode *inode = (Ext2_Inode *)(buffer + (inode_block_offset * extfs->inode_size)); 
  *out_inode = *inode;
 return 0;
}

static inline
int ext2fs_read_file_to_physical(const char *path, size_t path_length, uintptr_t physical){
  Ext2_Filesystem *extfs = &globals.ext2_filesystem; 

  if(path[0] != '/'){
    klog_error("malformed path.  Path's must begin with a '/'");
    return 0;
  }

  size_t index = 0;
  while(index < path_length){
    if(path[index] != '/'){
      klog_error("malformed path");
    }

    index++;
    const char *current = &path[index];
    while(path[index] != '/'){
      if(index >= path_length) break;
      index++;
    }

    size_t length = &path[index] - current;
    klog_debug("%.*s", length, current); 
  } 

  Ext2_Inode current_inode = {};
  ext2_read_inode(2, extfs, &current_inode); 
  uint8_t buffer[4096] = {};
  if(current_inode.type_and_permissions & EXT2_INODE_TYPE_DIRECTORY){
    klog_debug("directory_entries: %u", (uint32_t)current_inode.hard_link_count);
    uint16_t current_directory_index = 0;
    if(ext2fs_read_block(extfs, current_inode.direct_block_pointer_0, buffer) == 0){
      klog_error("failed to read directory entries");
      return 0;
    }

    Ext2_Directory_Entry *directory_entry = (Ext2_Directory_Entry *)buffer;
    while(((uintptr_t)directory_entry - (uintptr_t)buffer) < 4096 && (current_directory_index < current_inode.hard_link_count)){
      if(directory_entry->entry_size == 0) break;
      kdebug_ext2_log_directory_entry(extfs, directory_entry);
      directory_entry = (Ext2_Directory_Entry *)((uintptr_t)directory_entry + directory_entry->entry_size);
    } 
  }

  return 0;
}

