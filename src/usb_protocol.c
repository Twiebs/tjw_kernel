
static const uint32_t USB_REQUEST_TYPE_HOST_TO_DEVICE = 0x00;
static const uint32_t USB_REQUEST_TYPE_DEVICE_TO_HOST = 0x80;

static const uint32_t USB_REQUEST_GET_DESCRIPTION = 0x06;

//=========================================================

static const uint32_t USB_DESCRIPTOR_DEVICE = 0x01;
static const uint32_t USB_DESCRIPTOR_CONFIG = 0x02;
static const uint32_t USB_DESCRIPTOR_STRING = 0x03;

typedef struct {
  uint8_t length;
  uint8_t type;
  uint16_t usb_version;
  uint8_t device_class;
  uint8_t device_subclass;
  uint8_t device_protocol;
  uint8_t max_packet_size;
  uint16_t vendor_id;
  uint16_t product_id;
  uint16_t device_version;
  uint8_t vendor_string;
  uint8_t product_string;
  uint8_t serial_string;
  uint8_t config_count;
} __attribute((packed)) USB_Device_Descriptor;

typedef struct {

} __attribute((packed)) USB_Config_Descriptor;

typedef struct {
  uint8_t length;
  uint8_t type;
  uint16_t string[];
} __attribute((packed)) USB_String_Descriptor;

//===========================================================

typedef struct {
  uint8_t type;
  uint8_t request;
  uint16_t value;
  uint16_t index;
  uint16_t length;
} __attribute((packed)) USB_Device_Request;

#if 0
typedef struct {
  uint8_t packet_id;
  uint8_t address   : 7;
  uint8_t endpoint  : 4;
  uint8_t crc5 : 5;
} __attribute((packed)) USB_Token_Packet;
#endif

typedef struct {
  uint8_t packet_id;
} __attribute((packed)) USB_Handshake_Packet;