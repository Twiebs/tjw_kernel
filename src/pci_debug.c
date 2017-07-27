
static inline void pci_log_device_status(PCI_Device *pci_device){
  pci_set_config_address(pci_device->bus_number, pci_device->device_number, pci_device->function_number, 0x04);
  uint16_t status = 0, command = 0;
  pci_read_2x16(&status, &command);
  static const uint16_t PCI_STATUS_DETECTED_PARITY_ERROR = 1 << 15;
  static const uint16_t PCI_STATUS_SIGNALED_SYSTEM_ERROR = 1 << 14;
  static const uint16_t PCI_STATUS_RECEIVED_MASTER_ABORT = 1 << 13;
  static const uint16_t PCI_STATUS_RECEIVED_TARGET_ABORT = 1 << 12;
  static const uint16_t PCI_STATUS_SIGNALED_TARGET_ABORT = 1 << 11;
  static const uint16_t PCI_STATUS_MASTER_DATA_PARITY_ERROR = 1 << 8;
  klog_debug("detected_parity_error: %u", status & PCI_STATUS_DETECTED_PARITY_ERROR);
  klog_debug("signaled_system_error: %u", status & PCI_STATUS_SIGNALED_SYSTEM_ERROR);
  klog_debug("received_master_abort: %u", status & PCI_STATUS_RECEIVED_MASTER_ABORT);
  klog_debug("received_target_abort: %u", status & PCI_STATUS_RECEIVED_TARGET_ABORT);
  klog_debug("master_data_parity_error: %u", status & PCI_STATUS_MASTER_DATA_PARITY_ERROR); 
}

#if 0
static inline void pci_log_device_info(uint8_t bus_number, uint8_t slot_number){
  pci_set_config_address(bus_number, slot_number, 0, 0);
  uint16_t vendor, device;
  pci_read_2x16(&vendor, &device);
  if(vendor == 0xFFFF) { return; }

  pci_set_config_address(bus_number, slot_number, 0, 0x08);
  uint8_t class_code, subclass, prog_if, revision_id;
  pci_read_4x8(&class_code, &subclass, &prog_if, &revision_id);
  if(class_code == PCI_CLASS_SERIAL_BUS_CONTROLLER && subclass == PCI_SUBCLASS_USB_CONTROLLER){

    klog_debug("PCI Slot: 0x%X", (uint64_t)slot_number);
    klog_debug("  vendor: 0x%X", (uint64_t)vendor);
    klog_debug("  device: 0x%X", (uint64_t)device);
    klog_debug("  class_code: 0x%X", (uint64_t)class_code);
    klog_debug("  subclass: 0x%X", (uint64_t)subclass);
    klog_debug("  prog_if: 0x%X", (uint64_t)prog_if);



    pci_set_config_address(bus_number, slot_number, 0, 0x10);
    uint32_t usb_register_address = pci_read_uint32();
    klog_debug(" usb_register_address: 0x%X", usb_register_address);

    uintptr_t usb_virtual_page = 0x1000000; 
    uintptr_t page_offset = kmem_map_unaligned_physical_to_aligned_virtual_2MB(usb_register_address, usb_virtual_page, 0); 
    uintptr_t usb_virtual = usb_virtual_page + page_offset;
    ehci_initalize(usb_virtual); 
  }
}
#endif