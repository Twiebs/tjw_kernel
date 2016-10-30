
//NOTE(Torin 2016-10-15) Unpacked USB Request type bitmap fields
static const uint8_t USB_REQUEST_DIRECTION_DEVICE_TO_HOST = 0;
static const uint8_t USB_REQUEST_DIRECTION_HOST_TO_DEVICE = 1;
static const uint8_t USB_REQUEST_RECIPIENT_DEVICE = 0;
static const uint8_t USB_REQUEST_RECIPIENT_INTERFACE = 1;
static const uint8_t USB_REQUEST_RECIPIENT_ENDPOINT = 2;
static const uint8_t USB_REQUEST_RECIPIENT_OTHER = 3;
static const uint8_t USB_REQUEST_MODE_STANDARD = 0;
static const uint8_t USB_REQUEST_MODE_CLASS = 1;
static const uint8_t USB_REQUEST_MODE_VENDOR = 2;

//NOTE(Torin) USB Request ID List 
static const uint8_t USB_REQUEST_SET_ADDRESSS = 0x05;
static const uint8_t USB_REQUEST_GET_DESCRIPTOR = 0x06;
static const uint8_t USB_REQUEST_SET_CONFIGURATION = 0x09;

//NOTE(Torin) USB Descriptor Types
static const uint32_t USB_DESCRIPTOR_TYPE_DEVICE = 0x01;
static const uint32_t USB_DESCRIPTOR_TYPE_CONFIG = 0x02;
static const uint32_t USB_DESCRIPTOR_TYPE_STRING = 0x03;

static const uint16_t USB_DEVICE_CLASS_HUMAN_INTERFACE = 0x03;

//NOTE(Torin) USB String Descriptor Request LanguageID list
static const uint16_t USB_LANGID_ENGLISH_USA = 0x0409;

//Mass storage stuff
static const uint16_t USB_DEVICE_CLASS_MASS_STORAGE = 0x08;
static const uint8_t USB_MASS_STORAGE_PROTOCOL_BULK_ONLY = 0x50;

typedef struct {
  uint8_t descriptor_length;
  uint8_t descriptor_type;
} __attribute((packed)) USB_Descriptor_Common;

typedef struct {
  volatile uint8_t descriptor_length;
  volatile uint8_t descriptor_type;
  volatile uint16_t usb_version;
  volatile uint8_t device_class;
  volatile uint8_t device_subclass;
  volatile uint8_t device_protocol;
  volatile uint8_t max_packet_size;
  volatile uint16_t vendor_id;
  volatile uint16_t product_id;
  volatile uint16_t device_version;
  volatile uint8_t vendor_string;
  volatile uint8_t product_string;
  volatile uint8_t serial_string;
  volatile uint8_t config_count;
} __attribute((packed)) USB_Device_Descriptor;

typedef struct {
  uint8_t descriptor_length;
  uint8_t descriptor_type;
  uint16_t total_length;
  uint8_t interface_count;
  uint8_t configuration_value;
  uint8_t configuration_string;
  uint8_t attributes;
  uint8_t max_power;
} __attribute((packed)) USB_Configuration_Descriptor;

typedef struct {
  uint8_t descriptor_length;
  uint8_t descriptor_type;
  uint8_t interface_number;
  uint8_t alternate_setting;
  uint8_t endpoint_count;
  uint8_t interface_class;
  uint8_t interface_subclass;
  uint8_t interface_protocol;
  uint8_t interface_string_index;
} __attribute((packed)) USB_Interface_Descriptor;

typedef struct {
  uint8_t descriptor_length;
  uint8_t descriptor_type;
  //unpacked uint8_t endpoint_address 
  uint8_t endpoint_number    : 4; //0-3
  uint8_t reserved0          : 3; //4-6
  uint8_t endpoint_direction : 1; //7
  //unpacked uint8_t attributes
  uint8_t transfer_type     : 2; //0-1
  uint8_t synch_type        : 2; //2-3
  uint8_t usage_type        : 2; //4-5
  uint8_t reserved1         : 2; //6-7
  //=====================================
  uint16_t max_packet_size;
  uint8_t interval;
} __attribute((packed)) USB_Endpoint_Descriptor;

typedef struct {
  uint8_t length;
  uint8_t type;
  uint16_t string[0];
} __attribute((packed)) USB_String_Descriptor;

//===========================================================

typedef struct {
  //uint8_t RequestType unpacked
  volatile uint8_t recipient : 5;
  volatile uint8_t mode : 2;
  volatile uint8_t direction : 1;
  //=============================
  volatile uint8_t request;
  volatile uint8_t value_low;
  volatile uint8_t value_high;
  volatile uint8_t index_low;
  volatile uint8_t index_high;
  volatile uint16_t length;
} __attribute((packed)) USB_Device_Request;

static const uint32_t USB_CBW_SIGNATURE = 0x43425355;
static const uint32_t USB_CSW_SIGNATURE = 0x53425355;

typedef struct {
  uint32_t signature;
  uint32_t tag;
  uint32_t transfer_length;
  //NOTE(Torin) unpacked CBW Flags
  uint8_t reserved0 : 7;
  uint8_t direction : 1; //0:HostToDevice 1:DeviceToHost
  //================================================
  uint8_t logical_unit_number; //Max Value = 15
  uint8_t length; //Must be 1 <-> 16 (inclusive)
} __attribute((packed)) USB_Command_Block_Wrapper;

typedef struct {
  uint32_t signature;
  uint32_t tag;
  uint32_t data_residue;
  uint8_t status;
} __attribute((packed)) USB_Command_Status_Wrapper;

//=======================================================================

static const uint8_t SCSI_PAGE_CODE_RIGID_DRIVE_GEOMETRY = 0x04;

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

static const char *USB_REQUEST_NAMES[] = {
  "GET_STATUS",
  "CLEAR_FEATURE",
  "RESERVED",
  "SET_FEATURE",
  "RESERVED",
  "SET_ADDRESS",
  "GET_DESCRIPTOR",
  "GET_CONFIGURATION",
  "SET_CONFIGURATION",
  "GET_INTERFACE",
  "SET_INTERFACE",
  "SYNC_FRAME",
};

static const char *USB_DESCRIPTOR_NAMES[] = {
  "INVALID_DESCRIPTOR",
  "DEVICE",
  "CONFIGURATION",
  "STRING",
  "INTERFACE",
  "ENDPOINT",
  "DEVICE_QUALIFER",
  "OTHER_SPEED_CONFIGURATION",
  "INTERFACE_POWER",
};

//==========================================================================

static inline
void kdebug_log_usb_request(USB_Device_Request *request){
  const char *request_name = USB_REQUEST_NAMES[request->request];
  if(request->request == USB_REQUEST_GET_DESCRIPTOR){
    const char *descriptor_name = USB_DESCRIPTOR_NAMES[request->value_high];
    klog_debug("request: %s, value: %s", request_name, descriptor_name);
  } else {
     klog_debug("request: %s", request_name);
  }
}

static inline
void kdebug_log_usb_device_info(USB_Device *device){
  klog_debug("usb_device_info:");
  klog_debug("  device_address: 0x%X", (uint64_t)device->device_address);
  klog_debug("  device_class: 0x%X", (uint64_t)device->device_class);
  klog_debug("  vendor_id: 0x%X", (uint64_t)device->vendor_id);
  klog_debug("  product_id: 0x%X", (uint64_t)device->product_id);
  klog_debug("  vendor_string: %.*s", device->vendor_string_length, device->vendor_string);
  klog_debug("  product_string: %.*s", device->product_string_length, device->product_string);
}

static inline
void kdebug_log_usb_descriptor(void *descriptor_pointer){
  USB_Descriptor_Common *descriptor = (USB_Descriptor_Common *)descriptor_pointer;
  klog_debug("descriptor_length: %u", descriptor->descriptor_length);
  klog_debug("descriptor_type: %u", descriptor->descriptor_type);
  switch(descriptor->descriptor_type){
    case 0x01 /*USB_DESCRIPTOR_TYPE_DEVICE*/: {
      USB_Device_Descriptor *device_descriptor = (USB_Device_Descriptor *)descriptor_pointer;
      klog_debug("device_class: 0x%X", (uint64_t)device_descriptor->device_class);
      klog_debug("max_packet_size: %u", (uint32_t)device_descriptor->max_packet_size);
      klog_debug("vendor_id: 0x%X", (uint64_t)device_descriptor->vendor_id);
      klog_debug("product_id: 0x%X", (uint64_t)device_descriptor->product_id);
      klog_debug("vendor_string: 0x%X", (uint64_t)device_descriptor->vendor_string);
      klog_debug("product_string: 0x%X", (uint64_t)device_descriptor->product_string); 
    } break;
  }
}