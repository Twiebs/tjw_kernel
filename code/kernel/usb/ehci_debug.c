
void kdebug_log_ehci_operational_registers(EHCI_Operational_Registers *opregs){
  log_debug(EHCI, "usbcmd: 0x%X", (uint64_t)opregs->usb_command);
  log_debug(EHCI, "usbstd: 0x%X", (uint64_t)opregs->usb_status);
  log_debug(EHCI, "usbintr: 0x%X", (uint64_t)opregs->usb_interrupt);
  log_debug(EHCI, "frindex: 0x%X", (uint64_t)opregs->frame_index);
}

void kdebug_log_qtd_token(const uint32_t in_token){
  EHCI_QTD_Token qtd_token = *(EHCI_QTD_Token *)(&in_token);
  log_debug(EHCI, " split_transaction_state: %u", (uint32_t)qtd_token.split_transaction_state);
  log_debug(EHCI, " missed_micro_frame: %u", (uint32_t)qtd_token.missed_micro_frame);
  log_debug(EHCI, " transaction_error: %u", (uint32_t)qtd_token.transaction_error);
  log_debug(EHCI, " babble_detected: %u", (uint32_t)qtd_token.babble_detected);
  log_debug(EHCI, " data_buffer_error: %u", (uint32_t)qtd_token.data_buffer_error);
  log_debug(EHCI, " halted: %u", (uint32_t)qtd_token.halted);
  log_debug(EHCI, " active: %u", (uint32_t)qtd_token.active);
  log_debug(EHCI, " error_counter: %u", (uint32_t)qtd_token.error_counter);
  log_debug(EHCI, " data_toggle: %u", (uint32_t)qtd_token.data_toggle);
  log_debug(EHCI, " total_bytes_to_transfer: %u", (uint32_t)qtd_token.total_bytes_to_transfer);
}

void kdebug_log_qtd(const EHCI_QTD *qtd){
  log_debug(EHCI, " next_td: 0x%X", (uint64_t)qtd->next_td);
  log_debug(EHCI, " alt_next_td: 0x%X", (uint64_t)qtd->alt_next_td);
  uint64_t buffer_pointer = ((uint64_t)qtd->buffer_pointer_high[0] << 32) | qtd->buffer_pointer_low[0];
  log_debug(EHCI, " buffer_pointer: 0x%X", buffer_pointer);
  kdebug_log_qtd_token(qtd->qtd_token);
}

void kdebug_log_hc_status(const EHCI_Controller *hc){
  const EHCI_USB_Status_Register *usbsts = (const EHCI_USB_Status_Register *)&hc->op_regs->usb_status;
  log_debug(EHCI, "usb_interrupt: %u", (uint32_t)usbsts->usb_interrupt); 
  log_debug(EHCI, "usb_error_interrupt: %u", (uint32_t)usbsts->usb_error_interrupt);
  log_debug(EHCI, "port_change_detect: %u", (uint32_t)usbsts->port_change_detect);
  log_debug(EHCI, "frame_list_rollover: %u", (uint32_t)usbsts->frame_list_rollover);
  log_debug(EHCI, "host_system_error: %u", (uint32_t)usbsts->host_system_error);
  log_debug(EHCI, "interrupt_on_async_advance: %u", (uint32_t)usbsts->interrupt_on_async_advance);
  log_debug(EHCI, "hc_halted: %u", (uint32_t)usbsts->hc_halted);
  log_debug(EHCI, "reclamation: %u", (uint32_t)usbsts->reclamation);
  log_debug(EHCI, "perodic_schedule_status: %u", (uint32_t)usbsts->perodic_schedule_status);
  log_debug(EHCI, "asynch_schedule_status: %u", (uint32_t)usbsts->asynch_schedule_status);
}