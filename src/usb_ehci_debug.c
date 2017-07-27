
void kdebug_log_ehci_operational_registers(EHCI_Operational_Registers *opregs){
  klog_debug("usbcmd: 0x%X", (uint64_t)opregs->usb_command);
  klog_debug("usbstd: 0x%X", (uint64_t)opregs->usb_status);
  klog_debug("usbintr: 0x%X", (uint64_t)opregs->usb_interrupt);
  klog_debug("frindex: 0x%X", (uint64_t)opregs->frame_index);
}

void kdebug_log_qtd_token(const uint32_t in_token){
  EHCI_QTD_Token qtd_token = *(EHCI_QTD_Token *)(&in_token);
  klog_debug(" split_transaction_state: %u", (uint32_t)qtd_token.split_transaction_state);
  klog_debug(" missed_micro_frame: %u", (uint32_t)qtd_token.missed_micro_frame);
  klog_debug(" transaction_error: %u", (uint32_t)qtd_token.transaction_error);
  klog_debug(" babble_detected: %u", (uint32_t)qtd_token.babble_detected);
  klog_debug(" data_buffer_error: %u", (uint32_t)qtd_token.data_buffer_error);
  klog_debug(" halted: %u", (uint32_t)qtd_token.halted);
  klog_debug(" active: %u", (uint32_t)qtd_token.active);
  klog_debug(" error_counter: %u", (uint32_t)qtd_token.error_counter);
  klog_debug(" data_toggle: %u", (uint32_t)qtd_token.data_toggle);
  klog_debug(" total_bytes_to_transfer: %u", (uint32_t)qtd_token.total_bytes_to_transfer);
}

void kdebug_log_qtd(const EHCI_QTD *qtd){
  klog_debug(" next_td: 0x%X", (uint64_t)qtd->next_td);
  klog_debug(" alt_next_td: 0x%X", (uint64_t)qtd->alt_next_td);
  uint64_t buffer_pointer = ((uint64_t)qtd->buffer_pointer_high[0] << 32) | qtd->buffer_pointer_low[0];
  klog_debug(" buffer_pointer: 0x%X", buffer_pointer);
  kdebug_log_qtd_token(qtd->qtd_token);
}

void kdebug_log_hc_status(const EHCI_Controller *hc){
  const EHCI_USB_Status_Register *usbsts = (const EHCI_USB_Status_Register *)&hc->op_regs->usb_status;
  klog_debug("usb_interrupt: %u", (uint32_t)usbsts->usb_interrupt); 
  klog_debug("usb_error_interrupt: %u", (uint32_t)usbsts->usb_error_interrupt);
  klog_debug("port_change_detect: %u", (uint32_t)usbsts->port_change_detect);
  klog_debug("frame_list_rollover: %u", (uint32_t)usbsts->frame_list_rollover);
  klog_debug("host_system_error: %u", (uint32_t)usbsts->host_system_error);
  klog_debug("interrupt_on_async_advance: %u", (uint32_t)usbsts->interrupt_on_async_advance);
  klog_debug("hc_halted: %u", (uint32_t)usbsts->hc_halted);
  klog_debug("reclamation: %u", (uint32_t)usbsts->reclamation);
  klog_debug("perodic_schedule_status: %u", (uint32_t)usbsts->perodic_schedule_status);
  klog_debug("asynch_schedule_status: %u", (uint32_t)usbsts->asynch_schedule_status);
}