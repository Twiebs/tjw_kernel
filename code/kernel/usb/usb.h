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

int uhci_initalize_host_controller(uintptr_t hc_physical_address, PCI_Device *pci_device);
int ohci_initalize_host_controller(uintptr_t hc_physical_address, PCI_Device *pci_device);
int xhci_initalize_host_controller(uintptr_t hc_physical_address, PCI_Device *pci_device);

void kdebug_log_usb_mass_storage_device(USB_Mass_Storage_Device *usb_msd);