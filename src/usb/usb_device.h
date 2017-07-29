
typedef enum {
  USB_Device_Type_MASS_STORAGE,
} USB_Device_Type;

typedef enum {
  USB_Speed_FULL = 0b00,
  USB_Speed_LOW  = 0b01,
  USB_Speed_HIGH = 0b10,
} USB_Speed;

typedef struct {
  uint16_t vendor_id;
  uint16_t product_id;
  uint8_t device_address;
  uint8_t device_class;
  uint8_t vendor_string_length;
  uint8_t product_string_length;
  uint8_t vendor_string[128];
  uint8_t product_string[128];
  USB_Device_Type type;
  union {
    USB_Mass_Storage_Device msd;
  };
} USB_Device;

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