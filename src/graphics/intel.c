
//NOTE(Torin 2017-08-27) This procedure is called during pci_device initalization
//by the pci_device_driver information. The physical address parameters are obtained
//from the pci_device BAR registers.
Error_Code intel_graphics_device_initalize(uintptr_t base_physical_address) {
  static const uintptr_t GMBUS_AND_IO_CONTROLL_OFFSET = 0x5000;
  static const uintptr_t DISPLAY_PIPELINE_REGISTERS_OFFSET = 0x6000;
  uintptr_t gmbus_and_io_control_registers_physical_address = base_physical_address + GMBUS_AND_IO_CONTROLL_OFFSET;
  uintptr_t display_pipeline_registers_physical_address = base_physical_address + DISPLAY_PIPELINE_REGISTERS_OFFSET;
  uintptr_t gmbus_and_io_control_registers_virtual_address = memory_map_physical_mmio(gmbus_and_io_control_registers_physical_address, 1);
  uintptr_t display_pipeline_registers_virtual_address = memory_map_physical_mmio(display_pipeline_registers_physical_address, 1);

  //TODO(Torin 2017-08-24) Decide how this should be allocated
  Intel_Graphics_Device graphics_device = {};

  //NOTE(Torin 2017-08-24) We don't use a struct for the GMBUS registers because
  //the last register is 16 byte aligned instead of 4 aligned.  We just manualy
  //set pointers to the registers to avoid any confusion about how they are laid
  //out in memory, rather than relying on 4byte struct packing.
  static const uint64_t GMBUS0_OFFSET = 0x100;
  static const uint64_t GMBUS1_OFFSET = 0x104;
  static const uint64_t GMBUS2_OFFSET = 0x108;
  static const uint64_t GMBUS3_OFFSET = 0x10C;
  static const uint64_t GMBUS4_OFFSET = 0x110;
  static const uint64_t GMBUS5_OFFSET = 0x120;
  graphics_device.gmbus0 = (Intel_GMBUS0 *)(gmbus_and_io_control_registers_virtual_address + GMBUS0_OFFSET);
  graphics_device.gmbus1 = (Intel_GMBUS1 *)(gmbus_and_io_control_registers_virtual_address + GMBUS1_OFFSET);
  graphics_device.gmbus2 = (Intel_GMBUS2 *)(gmbus_and_io_control_registers_virtual_address + GMBUS2_OFFSET);
  graphics_device.gmbus3 = (Intel_GMBUS3 *)(gmbus_and_io_control_registers_virtual_address + GMBUS3_OFFSET);
  graphics_device.gmbus4 = (Intel_GMBUS4 *)(gmbus_and_io_control_registers_virtual_address + GMBUS4_OFFSET);
  graphics_device.gmbus5 = (Intel_GMBUS5 *)(gmbus_and_io_control_registers_virtual_address + GMBUS5_OFFSET);

  //Read EDID and get mode info
  //TODO(Torin 2017-08-27) Once more graphics drivers are wrriten determine if this
  //type of operation can be abstracted out for reuse by all drivers that obtian
  //display information from connected monitors/display devices
  static const uint8_t EDID_ADDRESS = 0x50;
  graphics_device.gmbus0->pair_pin_select = INTEL_GMBUS0_PIN_PAIR_SELECT_DDC;
  graphics_device.gmbus0->rate_select = INTEL_GMBUS0_RATE_SELECT_100KHZ;
  uint8_t offset = 0;
  //Display_Mode mode = {};
  Extended_Display_Identification edid = {};
  intel_graphics_device_i2c_write(&graphics_device, EDID_ADDRESS, 1, &offset);
  intel_graphics_device_i2c_read(&graphics_device, EDID_ADDRESS, 128, &edid);

  //Error_Code error = extract_display_mode_information(&edid, &mode);
  //if (error) return error;

  //NOTE(Torin 2017-08-27) Going to set the display mode.  Mabye change this to its
  //own procedure to handle resolutions switches later?
  //debug_log_display_mode(&mode);


  return Error_Code_NONE;
}

Error_Code intel_graphics_device_gmbus_wait_hardware_ready(Intel_Graphics_Device *graphics_device) {
  klog_debug("intel wait hardware ready");
  while (true) {
    if (graphics_device->gmbus2->nak_inticator) {
      klog_error("intel gmbus NAK");
      kernel_panic();
    }

    if (graphics_device->gmbus2->hardware_ready) {
      break;
    }
  }
  return Error_Code_NONE;
}

Error_Code intel_graphics_device_gmbus_wait_hardware_complete(Intel_Graphics_Device *graphics_device) {
  klog_debug("intel wait hardware complete");
  while (true) {
    if (graphics_device->gmbus2->nak_inticator) {
      klog_error("intel gmbus NAK");
      kernel_panic();
    }

    if (graphics_device->gmbus2->hardware_phase_wait) {
      break;
    }
  }
  return Error_Code_NONE;
}

Error_Code intel_graphics_device_i2c_write(Intel_Graphics_Device *graphics_device, uint16_t address, uint64_t size, void *data) {
  if (address >= (1 << 7)) {
    //TODO(Torin 2017-08-22) Implement this!
    klog_error("Address is to large for 7bits.  Must implement 10bit addressing!");
    return Error_Code_UNSUPORTED_FEATURE;
  }

  if (size >= (1 << 9)) {
    klog_error("Unsupported transfer size!");
    return Error_Code_INVALID_DATA;
  }

  uint8_t *byte_data = (uint8_t *)data;
  uint64_t bytes_written = 0;
  while (bytes_written < 4 && bytes_written < size) {
    graphics_device->gmbus3->data |= (byte_data[bytes_written] << (8 * bytes_written));
    bytes_written += 1;
  }

  
  //TODO(Torin 2017-08-22) This needs to be changed to a procedure that handles
  //10 bit addressing!
  graphics_device->gmbus1->slave_direction = INTEL_GMBUS1_SLAVE_DIRECTION_WRITE;
  graphics_device->gmbus1->slave_address = address;
  graphics_device->gmbus1->total_byte_count = size;
  graphics_device->gmbus1->bus_cycle_select = INTEL_GMBUS1_BUS_CYCLE_SELECT_WAIT;
  graphics_device->gmbus1->software_ready = 1;

  intel_graphics_device_gmbus_wait_hardware_ready(graphics_device);
  while (bytes_written < size) {
    size_t i = 0;
    for (i = 0; i < 4; i++) {
      if (bytes_written + i >= size) break;
      graphics_device->gmbus3->data |= (byte_data[bytes_written + i] << ( 8 * i));
    }

    bytes_written += i;
    intel_graphics_device_gmbus_wait_hardware_ready(graphics_device);
  }

  intel_graphics_device_gmbus_wait_hardware_complete(graphics_device);
  return Error_Code_NONE;
}

Error_Code intel_graphics_device_i2c_read(Intel_Graphics_Device *graphics_device, uint16_t address, uint64_t size, void *buffer) {
  if (address >= (1 << 7)) {
    //TODO(Torin 2017-08-22) Implement this!
    klog_error("Address is to large for 7bits.  Must implement 10bit addressing!");
    return Error_Code_UNSUPORTED_FEATURE;
  }

  if (size >= (1 << 9)) {
    klog_error("Unsupported transfer size!");
    return Error_Code_INVALID_DATA;
  }

  graphics_device->gmbus1->slave_direction = INTEL_GMBUS1_SLAVE_DIRECTION_READ;
  graphics_device->gmbus1->slave_address = address;
  graphics_device->gmbus1->total_byte_count = size;
  graphics_device->gmbus1->bus_cycle_select = INTEL_GMBUS1_BUS_CYCLE_SELECT_WAIT;
  graphics_device->gmbus1->software_ready = 1;

  uint8_t *byte_data = (uint8_t *)buffer;
  size_t bytes_read = 0;
  while (bytes_read < size) {
    intel_graphics_device_gmbus_wait_hardware_ready(graphics_device);
    uint32_t data = graphics_device->gmbus3->data;
    for (size_t i = 0; i < 4; i++) {
      if (bytes_read + i >= size) break;
      byte_data[bytes_read + i] = data >> (8 * i);
    }

    bytes_read += 4;
  }

  intel_graphics_device_gmbus_wait_hardware_complete(graphics_device);
  return Error_Code_NONE;
}

void intel_graphics_device_dac_enable(Intel_Graphics_Device *graphics_device) {
  static const uint32_t DAC_ENABLED_BIT = 1 << 31;
}

void intel_graphics_device_dac_disable(Intel_Graphics_Device *graphics_device);