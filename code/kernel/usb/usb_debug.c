void kdebug_log_usb_request(USB_Device_Request *request){
  const char *request_name = USB_REQUEST_NAMES[request->request];
  if(request->request == USB_REQUEST_GET_DESCRIPTOR){
    const char *descriptor_name = USB_DESCRIPTOR_NAMES[request->value_high];
    klog_debug("request: %s, value: %s", request_name, descriptor_name);
  } else {
     klog_debug("request: %s", request_name);
  }
}

void kdebug_log_usb_device_info(USB_Device *device){
  klog_debug("usb_device_info:");
  klog_debug("  device_address: 0x%X", (uint64_t)device->device_address);
  klog_debug("  device_class: 0x%X", (uint64_t)device->device_class);
  klog_debug("  vendor_id: 0x%X", (uint64_t)device->vendor_id);
  klog_debug("  product_id: 0x%X", (uint64_t)device->product_id);
  klog_debug("  vendor_string: %.*s", device->vendor_string_length, device->vendor_string);
  klog_debug("  product_string: %.*s", device->product_string_length, device->product_string);
}

void kdebug_log_usb_descriptor(void *descriptor_pointer){
  USB_Descriptor_Common *descriptor = (USB_Descriptor_Common *)descriptor_pointer;
  klog_debug("descriptor_length: %u", descriptor->descriptor_length);
  klog_debug("descriptor_type: %u", descriptor->descriptor_type);
  switch(descriptor->descriptor_type){
    case 0x01 /*USB_DESCRIPTOR_TYPE_DEVICE*/: {
      USB_Device_Descriptor *device_descriptor = (USB_Device_Descriptor *)descriptor_pointer;
      klog_debug("device_class: 0x%X", (uint64_t)device_descriptor->device_class);
      klog_debug("max_packet_size: %u", (uint32_t)device_descriptor->max_packet_size);
      klog_debug("vendor_id: 0x%X", (uint64_t)device_descriptor->vendor_id);
      klog_debug("product_id: 0x%X", (uint64_t)device_descriptor->product_id);
      klog_debug("vendor_string: 0x%X", (uint64_t)device_descriptor->vendor_string);
      klog_debug("product_string: 0x%X", (uint64_t)device_descriptor->product_string); 
    } break;
  }
}

void kdebug_log_usb_mass_storage_device(USB_Mass_Storage_Device *usb_msd) {
  klog_debug("USB_Mass_Storage_Device:");
  klog_debug(" logical_block_size: %u", usb_msd->logical_block_size);
  klog_debug(" logical_block_count: %lu", usb_msd->logical_block_count);
}