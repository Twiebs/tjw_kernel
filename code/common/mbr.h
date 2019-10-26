
// https://wiki.osdev.org/MBR_(x86)

#include <stdint.h>

static const uint16_t MBR_VALID_BOOTSECTOR_SIGNATURE = 0xaa55;

typedef struct MBR_Partition_Table
{
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

typedef struct MBR
{
    uint8_t bootstrap_code[440];
    uint32_t unique_disk_id;
    uint16_t reserved;
    MBR_Partition_Table partition_table[4];
    uint16_t valid_bootsector_signature;
} __attribute((packed)) MBR;