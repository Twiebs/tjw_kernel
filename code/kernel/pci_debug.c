
void pci_log_device_status(PCI_Device *pci_device){
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