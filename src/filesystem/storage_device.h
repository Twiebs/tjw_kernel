
typedef enum {
  MBR_Partition_Type_LINUX_FILESYSTEM = 0x83,
} MBR_Partition_Type;

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
  File_System_Type file_system_type;
  uint64_t first_block;
  uint64_t block_count;
} Storage_Device_Partition;

typedef enum {
  Storage_Device_Type_INVALID,
  Storage_Device_Type_EHCI,
  Storage_Device_Type_XHCI,
  Storage_Device_Type_SATA,
} Storage_Device_Type;

typedef struct {
  Storage_Device_Type type;
  Storage_Device_Partition partitions[4];
  uint64_t partition_count;
  uint64_t block_size;
  void *device_ptr;
  void *controller_ptr;
} Storage_Device;

int storage_device_initalize(Storage_Device *device);
int storage_device_read_to_physical(Storage_Device *device, uint64_t block_number, uint64_t block_count, uintptr_t physical_address);