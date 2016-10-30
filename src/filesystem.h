
typedef struct {
  uint8_t partition_type;
  uint64_t first_block;
  uint64_t block_count;
} Partition_Info;

typedef enum {
  Storage_Device_INVALID,
  Storage_Device_EHCI,
  Storage_Device_XHCI,
  Storage_Device_SATA,
} Storage_Device_Type;

typedef struct {
  //TODO(Torin 2016-10-29) Yup... Past Torin is this fucking stupid.  Please don't be mad.
  Storage_Device_Type type;
  void *device_ptr;
  void *controller_ptr;
} Storage_Device;

typedef struct {
  uint32_t inode_count_per_group;
  uint32_t block_count_per_group;
  uint32_t inode_size;
  uint32_t block_size;
  uint32_t sectors_per_block;
  uint32_t partition_first_sector;
  uint32_t superblock_sector;
  uint32_t required_features;
  uint32_t root_inode;
  Storage_Device storage_device;
} Ext2_Filesystem;