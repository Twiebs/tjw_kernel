
static const uint8_t SCSI_PAGE_CODE_RIGID_DRIVE_GEOMETRY = 0x04;

#include "usb_debug.c"

typedef struct {
  USB_Command_Block_Wrapper cbw;
  uint8_t operation_code; //0x12
  //=============================
  uint8_t enable_vital_product_data : 1;
  uint8_t optional_command_support_data : 1;
  uint8_t reserved0 : 6;
  //=============================
  uint8_t page_code;
  uint8_t reserved1;
  uint8_t allocation_length;
  uint8_t control;
  uint8_t padding[10];
} __attribute((packed)) SCSI_Inquiry_Command;

typedef struct {
  USB_Command_Block_Wrapper cbw;
  uint8_t operation_code; //0x00 
  uint8_t reserved_0;
  uint8_t reserved_1;
  uint8_t reserved_2;
  uint8_t reserved_3;
  uint8_t control;
  uint8_t padding[10];
} __attribute((packed)) SCSI_Test_Unit_Ready_Command;

typedef struct {
  USB_Command_Block_Wrapper cbw;
  uint8_t operation_code; //0x25
  uint8_t reserved_0;
  uint8_t logical_block_address_3;
  uint8_t logical_block_address_2;
  uint8_t logical_block_address_1;
  uint8_t logical_block_address_0;
  uint8_t reserved_1;
  uint8_t reserved_2;
  uint8_t pmi : 1;
  uint8_t reserved_3 : 7;
  uint8_t control;
  uint8_t padding[6];
} __attribute((packed)) SCSI_Read_Capacity_Command;

typedef struct {
  USB_Command_Block_Wrapper cbw;
  uint8_t operation_code; //0x1A
  uint8_t reserved_0 : 3;
  uint8_t disable_block_descriptors : 1;
  uint8_t reserved_1 : 4;
  uint8_t page_code : 6;
  uint8_t page_control : 2;
  uint8_t reserved_2;
  uint8_t allocation_length;
  uint8_t control;
  uint8_t padding[6];
} __attribute((packed)) SCSI_Mode_Sense_Command;

typedef struct {
  USB_Command_Block_Wrapper cbw;
  uint8_t operation_code; //0x28
  //==============================
  uint8_t obsolete_0 : 1;
  uint8_t force_unit_access_non_volatile_cache : 1;
  uint8_t reserved_0 : 1;
  uint8_t force_unit_access : 1;
  uint8_t disable_page_out  : 1;
  uint8_t read_protect : 3;
  //=============================
  uint8_t logical_block_address_3;
  uint8_t logical_block_address_2;
  uint8_t logical_block_address_1;
  uint8_t logical_block_address_0;
  //==============================
  uint8_t group_number : 5;
  uint8_t reserved_1   : 3;
  //==============================
  uint8_t transfer_length_1;
  uint8_t transfer_length_0;
  uint8_t control;
  uint8_t padding[6];
} __attribute((packed)) SCSI_Read_Command;

static_assert(sizeof(SCSI_Inquiry_Command) == 31);
static_assert(sizeof(SCSI_Test_Unit_Ready_Command) == 31);
static_assert(sizeof(SCSI_Read_Capacity_Command) == 31);
static_assert(sizeof(SCSI_Read_Command) == 31);

typedef struct {
  uint8_t page_code : 6;
  uint8_t reserved_0 : 1;
  uint8_t parameter_savable : 1;
  //=============================
  uint8_t page_length;
  uint8_t cylinder_count_2;
  uint8_t cylinder_count_1;
  uint8_t cylinder_count_3;
  uint8_t head_count;
  //NOTE(Torin) The following data layout is unclear in spec but is unused by the kernel
  //See SCSI Command Reference Manual Page 312
  uint8_t starting_cylinder_write_precomp_2;
  uint8_t starting_cylinder_write_precomp_1;
  uint8_t starting_cylinder_write_precomp_0;
  uint8_t starting_cylinder_reduced_write_current_2;
  uint8_t starting_cylinder_reduced_write_current_1; 
  uint8_t starting_cylinder_reduced_write_current_0;
  uint8_t drive_step_rate_1;
  uint8_t drive_step_rate_0;
  uint8_t landing_zone_cylinder_2;
  uint8_t landing_zone_cylinder_1;
  uint8_t landing_zone_cylinder_0;
  //NOTE(Torin) The data layout is clear from here foward
  //====================================
  uint8_t rotational_position_locking : 2;
  uint8_t reserved_1 : 6;
  //====================================
  uint8_t rotational_offset;
  uint8_t reserved_2;
  uint8_t medium_rotation_rate_1;
  uint8_t medium_rotation_rate_0;
  uint8_t reserved_3;
  uint8_t reserved_4;
} __attribute((packed)) SCSI_Rigid_Drive_Geometry_Data;

typedef struct {
  uint8_t logical_block_address_3;
  uint8_t logical_block_address_2;
  uint8_t logical_block_address_1;
  uint8_t logical_block_address_0;
  uint8_t block_size_3;
  uint8_t block_size_2;
  uint8_t block_size_1;
  uint8_t block_size_0;
} __attribute((packed)) SCSI_Read_Capacity_Data;

typedef struct {
  uint8_t peripheral_device_type : 5; //0-4
  uint8_t peripheral_qualifier   : 3; //5-7
  uint8_t reserved0 : 7; 
  uint8_t rmb : 1;
  uint8_t version;
  //==============================
  uint8_t response_data_format : 4;
  uint8_t hisup : 1;
  uint8_t normaca : 1;
  uint8_t reserved1 : 2;
  //==================================
  uint8_t additional_length;
  //================================
  uint8_t protect : 1;
  uint8_t reserved2 : 2;
  uint8_t threepc : 1;
  uint8_t tpgs : 2;
  uint8_t acc : 1;
  uint8_t sccs : 1;
  //==============================
  uint8_t addr16 : 1;
  uint8_t obsolete0 : 2;
  uint8_t mchngr : 1;
  uint8_t multip : 1;
  uint8_t vs : 1;
  uint8_t encserv : 1;
  uint8_t bque : 1;
  //========================================
  uint8_t vs_1 : 1;
  uint8_t cmd_que : 1;
  uint8_t obsolete_1 : 1;
  uint8_t linked : 1;
  uint8_t sync : 1;
  uint8_t wbus16 : 1;
  uint8_t obsolete_2 : 2;
  uint8_t vendor_identification_1;
  uint8_t vendor_identification_0;
  uint8_t product_identification_1;
  uint8_t product_identification_0;
  uint8_t product_revision_level_1;
  uint8_t product_revision_level_0;
  uint8_t drive_serial_number_1;
  uint8_t drive_serial_number_0;
} __attribute((packed)) SCSI_Inquiry_Data;