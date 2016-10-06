
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
