
typedef enum {
  PCI_Device_Type_INVALID,
  PCI_Device_Type_UHCI,
  PCI_Device_Type_OHCI,
  PCI_Device_Type_EHCI,
  PCI_Device_Type_XHCI,
} PCI_Device_Type;

//TODO(Torin: 2017-07-27) What is this exactly
typedef struct {
  uint8_t reserved0;
  uint8_t function_number;
  uint8_t device_number;
  uint8_t bus_number;
} PCI_Device;

typedef struct {
  uintptr_t uhci_physical_address;
  uintptr_t ohci_physical_address;
  uintptr_t ehci_physical_address;
  uintptr_t xhci_physical_address;
  PCI_Device uhci_pci_device;
  PCI_Device ohci_pci_device;
  PCI_Device ehci_pci_device;
  PCI_Device xhci_pci_device;
} PCI_Info;