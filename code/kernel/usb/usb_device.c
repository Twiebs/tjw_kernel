
//TODO(Torin: 2017-07-27) Switch to this
void initalize_usb_mass_storage_device(USB_Device *device) {
  USB_Mass_Storage_Device *msd = &device->msd;
  USB_Device_Request msd_reset_request = {};
  msd_reset_request.recipient = USB_REQUEST_RECIPIENT_INTERFACE;
  msd_reset_request.mode = USB_REQUEST_MODE_CLASS;
  msd_reset_request.request = 0xFF;
  msd_reset_request.index_high = msd->interface_number >> 8;
  msd_reset_request.index_low = msd->interface_number;
}