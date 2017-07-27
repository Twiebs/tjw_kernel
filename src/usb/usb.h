typedef struct {
  uint8_t device_number;
  uint8_t interface_number;
  uint8_t out_endpoint;
  uint8_t in_endpoint;
  uint8_t out_toggle_value;
  uint8_t in_toggle_value;
  uint16_t out_endpoint_max_packet_size;
  uint16_t in_endpoint_max_packet_size;
  uint32_t logical_block_size;
  uint64_t logical_block_count;
} USB_Mass_Storage_Device;

typedef enum {
  USB_Device_Type_MASS_STORAGE,
} USB_Device_Type;

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

typedef enum {
  USB_Speed_FULL = 0b00,
  USB_Speed_LOW  = 0b01,
  USB_Speed_HIGH = 0b10,
} USB_Speed;

int uhci_initalize_host_controller(uintptr_t hc_physical_address, PCI_Device *pci_device);
int ohci_initalize_host_controller(uintptr_t hc_physical_address, PCI_Device *pci_device);
int ehci_initalize_host_controller(uintptr_t hc_physical_address, PCI_Device *pci_device);
int xhci_initalize_host_controller(uintptr_t hc_physical_address, PCI_Device *pci_device);