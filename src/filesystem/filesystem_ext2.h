
typedef struct {
  uint32_t inode_count_per_group;
  uint32_t block_count_per_group;
  uint32_t group_count;
  uint32_t inode_size;
  uint32_t block_size;
  uint32_t sectors_per_block;
  uint32_t partition_first_sector;
  uint32_t superblock_sector;
  uint32_t required_features;
  uint32_t root_inode;
  Storage_Device *storage_device;
} Ext2_Filesystem;

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
  uint16_t permissions : 12;
  uint16_t type : 4;
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
  uint32_t direct_block_pointers[12];
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

static const uint16_t EXT2_INODE_TYPE_FIFO              = 0b01;
static const uint16_t EXT2_INODE_TYPE_CHARACTER_DEVICE  = 0b10;
static const uint16_t EXT2_INODE_TYPE_DIRECTORY         = 0b100;
static const uint16_t EXT2_INODE_TYPE_BLOCK_DEVICE      = 0b110;
static const uint16_t EXT2_INODE_TYPE_REGULAR_FILE      = 0b1000;
static const uint16_t EXT2_INODE_TYPE_SYMBOLIC_LINK     = 0b1010; 
static const uint16_t EXT2_INODE_TYPE_UNIX_SOCKET       = 0b1100;

static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_UNKNOWN_TYPE     = 0;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_REGULAR_FILE     = 1;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_DIRECTORY        = 2;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_CHARACTER_DEVICE = 3;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_BLOCK_DEVICE     = 4;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_FIFO             = 5;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_SOCKET           = 6;
static const uint8_t EXT2_DIRECTORY_ENTRY_TYPE_SYMBOLIC_LINK    = 7;

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

bool ext2fs_read_block(Ext2_Filesystem *extfs, uint32_t block_number, uintptr_t buffer_physical);
int ext2fs_read_inode(Ext2_Filesystem *extfs, uint32_t inode_number, Ext2_Inode *out_inode);

int ext2_file_system_initalize(Ext2_Filesystem *extfs, Storage_Device *storage_device, uint64_t partition_index);