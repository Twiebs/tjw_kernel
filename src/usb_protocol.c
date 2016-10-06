
static const uint32_t USB_REQUEST_TYPE_HOST_TO_DEVICE = 0x00;
static const uint32_t USB_REQUEST_TYPE_DEVICE_TO_HOST = 0x80;
static const uint32_t USB_REQUEST_TYPE_RECIPIENT_ENDPOINT = 0b10;


//NOTE(Torin) USB Requests  
static const uint8_t USB_REQUEST_SET_ADDRESSS = 0x05;
static const uint8_t USB_REQUEST_GET_DESCRIPTOR = 0x06;
static const uint8_t USB_REQUEST_SET_CONFIGURATION = 0x09;

//=========================================================

static const uint32_t USB_DESCRIPTOR_TYPE_DEVICE = 0x01;
static const uint32_t USB_DESCRIPTOR_TYPE_CONFIG = 0x02;
static const uint32_t USB_DESCRIPTOR_TYPE_STRING = 0x03;

static const uint16_t USB_DEVICE_CLASS_HUMAN_INTERFACE = 0x03;

//Mass storage stuff
static const uint16_t USB_DEVICE_CLASS_MASS_STORAGE = 0x08;
static const uint8_t USB_MASS_STORAGE_PROTOCOL_BULK_ONLY = 0x50;

typedef struct {
  uint8_t device_number;
  uint8_t interface_number;
  uint8_t out_endpoint;
  uint8_t in_endpoint;
  uint8_t out_toggle_value;
  uint8_t in_toggle_value;
  uint16_t out_endpoint_max_packet_size;
  uint16_t in_endpoint_max_packet_size;
} USB_Mass_Storage_Device;

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
  volatile uint8_t type;
  volatile uint8_t request;
  volatile uint8_t value_low;
  volatile uint8_t value_high;
  volatile uint16_t index;
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


typedef struct {
  USB_Command_Block_Wrapper cbw;
  uint8_t operation_code; //0x12
  uint8_t enable_vital_product_data : 1;
  uint8_t reserved0 : 7;
  uint8_t page_code;
  uint8_t allocation_length_1;
  uint8_t allocation_length_0;
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

static inline
void scsi_create_inquiry(SCSI_Inquiry_Command *inquiry, const uint16_t length){
  static const uint8_t SCSI_COMMAND_INQUIRY = 0x12;
  inquiry->cbw.signature = USB_CBW_SIGNATURE;
  inquiry->cbw.tag = 0xFFFF0000; //TODO(Torin 2016-10-06) This can be whatever I want???
  inquiry->cbw.transfer_length = length;
  inquiry->cbw.direction = 1; //NOTE(Torin) Device to host
  inquiry->cbw.length = 6;
  inquiry->operation_code = SCSI_COMMAND_INQUIRY;
  inquiry->allocation_length_1 = (length >> 8) & 0xFF;
  inquiry->allocation_length_0 = (length & 0xFF);
}



#if 0
typedef struct {
  uint8_t operation_code;
  uint8_t reserved0 : 5;
  uint8_t misc_cdb_info : 3;
  uint8_t logical_block_address_high; //NOTE(Torin) Big Endian!!! This order is correct!
  uint8_t logical_block_address_low;
  uint8_t transfer_length;
  uint8_t control;
} __attribute((packed)) SCSI_Command_Descriptor_Block_6;
typedef struct {
  uint8_t operation_code;
  uint8_t service_action : 5;
  uint8_t misc_cdb_info_0 : 3;
  uint8_t logical_block_address_3;
  uint8_t logical_block_address_2;
  uint8_t logical_block_address_1;
  uint8_t logical_block_address_0;
  uint8_t misc_cdb_info_1;
  uint8_t length_1;
  uint8_t length_0;
  uint8_t control;
} __attribute((packed)) SCSI_Command_Descriptor_Block_10;
static_assert(sizeof(SCSI_Command_Descriptor_Block_6) == 6);
static_assert(sizeof(SCSI_Command_Descriptor_Block_10) == 10);
#endif


