
typedef enum {
  PCI_DEVICE_TYPE_INVALID,
  PCI_DEVICE_TYPE_UHCI,
  PCI_DEVICE_TYPE_OHCI,
  PCI_DEVICE_TYPE_EHCI,
  PCI_DEVICE_TYPE_XHCI,
} PCI_Device_Type;

typedef struct {
  uint8_t reserved0;
  uint8_t function_number;
  uint8_t device_number;
  uint8_t bus_number;
} PCI_Device;