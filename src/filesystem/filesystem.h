
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
  uint64_t fs_reference;
  uint64_t file_size;
  uint64_t creation_time;
  uint64_t accessed_time;
  uint64_t modified_time; 
} File_Handle;

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

static const uint8_t PARTITION_TYPE_LINUX_FILESYSTEM = 0x83;

int fs_read_file(File_Handle *h, uint64_t offset, uint64_t size, uintptr_t *physical_pages);
int fs_obtain_file_handle(const char *path, uint64_t path_length, File_Handle *in_handle);