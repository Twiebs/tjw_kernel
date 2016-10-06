
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
  uint32_t relative_sector;
  uint32_t total_sectors;
} __attribute((packed)) Partition_Table;