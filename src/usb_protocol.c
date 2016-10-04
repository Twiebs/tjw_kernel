
static const uint32_t USB_REQUEST_TYPE_HOST_TO_DEVICE = 0x00;
static const uint32_t USB_REQUEST_TYPE_DEVICE_TO_HOST = 0x80;
static const uint32_t USB_REQUEST_TYPE_RECIPIENT_ENDPOINT = 0b10;


static const uint32_t USB_REQUEST_SET_ADDRESSS = 0x05;
static const uint32_t USB_REQUEST_GET_DESCRIPTOR = 0x06;

//=========================================================

static const uint32_t USB_DESCRIPTOR_TYPE_DEVICE = 0x01;
static const uint32_t USB_DESCRIPTOR_TPYE_CONFIG = 0x02;
static const uint32_t USB_DESCRIPTOR_TYPE_STRING = 0x03;

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
