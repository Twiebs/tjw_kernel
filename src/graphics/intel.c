
//NOTE(Torin 2017-09-03)This procedure is called when the PCI graphics device is initialized.
//The register_physical_address paramater is obtained from the PCI configuration
//space BAR0 address. 
Error_Code intel_graphics_device_initialize(PCI_Device *pci_device)  {
  uintptr_t base_physical_address = pci_device_get_base_address_0(pci_device);
  static const uintptr_t GMBUS_AND_IO_CONTROLL_OFFSET = 0x5000;
  //static const uintptr_t DISPLAY_PIPELINE_REGISTERS_OFFSET = 0x6000;
  uintptr_t gmbus_and_io_control_registers_physical_address = base_physical_address + GMBUS_AND_IO_CONTROLL_OFFSET;
  //uintptr_t display_pipeline_registers_physical_address = base_physical_address + DISPLAY_PIPELINE_REGISTERS_OFFSET;
  uintptr_t gmbus_and_io_control_registers_virtual_address = memory_map_physical_mmio(gmbus_and_io_control_registers_physical_address, 1);
  //uintptr_t display_pipeline_registers_virtual_address = memory_map_physical_mmio(display_pipeline_registers_physical_address, 1);

  //TODO(Torin 2017-08-24) Decide how this should be allocated
  Intel_Graphics_Device graphics_device = {};
  graphics_device.mmio_registers_address = base_physical_address;

  //NOTE(Torin 2017-08-24) We don't use a struct for the GMBUS registers because
  //the last register is 16 byte aligned instead of 4 aligned.  We just manualy
  //set pointers to the registers to avoid any confusion about how they are laid
  //out in memory, rather than relying on 4byte struct packing.
  static const uint64_t GMBUS0_OFFSET = 0x100;
  graphics_device.gmbus_registers = (Intel_GMBUS_Registers *)(gmbus_and_io_control_registers_virtual_address + GMBUS0_OFFSET);

  {
    //Soft reset the GMBUS controller
    Intel_GMBUS1 gmbus1 = graphics_device.gmbus_registers->gmbus1;
    gmbus1.software_clear_interrupt = 1;
    graphics_device.gmbus_registers->gmbus1 = gmbus1;
    intel_graphics_device_gmbus_wait_hardware_ready(&graphics_device);
    gmbus1 = graphics_device.gmbus_registers->gmbus1;
    gmbus1.software_clear_interrupt = 0;
    graphics_device.gmbus_registers->gmbus1 = gmbus1;
    intel_graphics_device_gmbus_wait_hardware_ready(&graphics_device);

    //Set target pin and clock rate
    Intel_GMBUS0 gmbus0 = graphics_device.gmbus_registers->gmbus0;
    gmbus0.pair_pin_select = INTEL_GMBUS0_PIN_PAIR_SELECT_DDC;
    gmbus0.rate_select = INTEL_GMBUS0_RATE_SELECT_50KHZ;
    graphics_device.gmbus_registers->gmbus0 = gmbus0;
  }

  //Read EDID and get mode info
  uint8_t offset = 0;
  static const uint8_t EDID_ADDRESS = 0x50;
  Extended_Display_Identification edid = {};
  if (intel_graphics_device_i2c_write(&graphics_device, EDID_ADDRESS, 1, &offset)) {
    klog_error("failed intel graphics device i2c write for obtaining EDID");
    return Error_Code_INVALID_DATA;
  }
  
  if (intel_graphics_device_i2c_read(&graphics_device, EDID_ADDRESS, 128, &edid)) {
    klog_error("failed intel_graphics device i2c read to obtain EDID");
    return Error_Code_INVALID_DATA;
  }

#if 0
  if (edid.signature != EDID_SIGNATURE) {
    klog_error("EDID signature is invalid!: 0x%X", edid.signature);
    return Error_Code_INVALID_DATA;
  }
#endif

  Display_Mode mode = {};
  if (extract_display_mode_information(&edid, &mode)) {
    klog_error("couldn't extract display mode information!");
    return Error_Code_INVALID_DATA;
  }

  debug_log_display_mode(&mode);

  return Error_Code_NONE;
}

Error_Code intel_graphics_device_gmbus_wait_hardware_ready(Intel_Graphics_Device *graphics_device) {
  uint64_t garbage_timeout = 0xFFFFFF;
  uint64_t total_time_elapsed = 0x00;
  while (true) {
    Intel_GMBUS2 gmbus2 = graphics_device->gmbus_registers->gmbus2;
    if (gmbus2.nak_inticator) {
      klog_error("intel gmbus NAK");
      kernel_panic();
    }

    if (gmbus2.hardware_ready) {
      break;
    }

    if (total_time_elapsed >= garbage_timeout) {
      klog_error("GMBUS WAIT FOR READY TIMEOUT");
      return Error_Code_TIMEOUT;
    }
    total_time_elapsed += garbage_timeout;
  }
  return Error_Code_NONE;
}

Error_Code intel_graphics_device_gmbus_wait_hardware_complete(Intel_Graphics_Device *graphics_device) {
  uint64_t garbage_timeout = 0xFFFFFF;
  uint64_t total_time_elapsed = 0x00;
  while (true) {
    Intel_GMBUS2 gmbus2 = graphics_device->gmbus_registers->gmbus2;
    if (gmbus2.nak_inticator) {
      klog_error("intel gmbus NAK");
      kernel_panic();
    }

    if (gmbus2.hardware_phase_wait) {
      break;
    }

    if (total_time_elapsed >= garbage_timeout) {
      klog_error("GMBUS WAIT FOR COMPLETE TIMEOUT");
      return Error_Code_TIMEOUT;
    }

    total_time_elapsed += garbage_timeout;
  }
  return Error_Code_NONE;
}

Error_Code intel_graphics_device_gmbus_wait_hardware_stop(Intel_Graphics_Device *graphics_device) {
  uint64_t garbage_timeout = 0xFFFFFF;
  uint64_t total_time_elapsed = 0x00;
  while (true) {
    Intel_GMBUS2 gmbus2 = graphics_device->gmbus_registers->gmbus2;
    if (gmbus2.nak_inticator) {
      klog_error("intel gmbus NAK");
      kernel_panic();
    }

    if (gmbus2.hardware_phase_wait == 0) {
      break;
    }

    if (total_time_elapsed >= garbage_timeout) {
      klog_error("GMBUS WAIT FOR COMPLETE TIMEOUT");
      return Error_Code_TIMEOUT;
    }

    total_time_elapsed += garbage_timeout;
  }
  return Error_Code_NONE;
}

Error_Code intel_graphics_device_i2c_write(Intel_Graphics_Device *graphics_device, uint16_t address, uint64_t size, void *buffer) {
  if (address >= (1 << 7)) {
    //TODO(Torin 2017-08-22) Implement this!
    klog_error("Address is to large for 7bits.  Must implement 10bit addressing!");
    return Error_Code_UNSUPORTED_FEATURE;
  }

  if (size >= (1 << 9)) {
    klog_error("Unsupported transfer size!");
    return Error_Code_INVALID_DATA;
  }

  uint8_t *byte_data = (uint8_t *)buffer;
  uint64_t bytes_written = 0;
  uint8_t bytes_to_write[4];
  while (bytes_written < 4) {
    memory_set(bytes_to_write, 0x00, 4);
    if (bytes_written >= size) break;
    bytes_to_write[bytes_written] = byte_data[bytes_written];
    bytes_written += 1;
  }

  graphics_device->gmbus_registers->gmbus3.data = *(uint32_t *)bytes_to_write;

  {
    //TODO(Torin 2017-08-22) This needs to be changed to a procedure that handles
    //10 bit addressing!
    Intel_GMBUS1 gmbus1 = graphics_device->gmbus_registers->gmbus1;
    gmbus1.slave_direction = INTEL_GMBUS1_SLAVE_DIRECTION_WRITE;
    gmbus1.slave_address = address;
    gmbus1.total_byte_count = size;
    gmbus1.bus_cycle_select = INTEL_GMBUS1_BUS_CYCLE_SELECT_WAIT;
    gmbus1.software_ready = 1;
    graphics_device->gmbus_registers->gmbus1 = gmbus1;
    intel_graphics_device_gmbus_wait_hardware_ready(graphics_device);
  }

#if 0
  while (bytes_written < size) {
    memory_set(bytes_to_write, 0x00, 4);
    for (size_t i = 0; i < 4; i++) {
      if (bytes_written + 1 >= size) break;
      bytes_to_write[bytes_written] = byte_data[bytes_written];
      bytes_written += 1;
    }

    graphics_device->gmbus_registers->gmbus3.data = *(uint32_t *)bytes_to_write;
    intel_graphics_device_gmbus_wait_hardware_ready(graphics_device);
  }
#endif

  intel_graphics_device_gmbus_wait_hardware_complete(graphics_device);

  {
    Intel_GMBUS1 gmbus1 = graphics_device->gmbus_registers->gmbus1;
    gmbus1.bus_cycle_select = INTEL_GMBUS1_BUS_CYCLE_SELECT_STOP;
    graphics_device->gmbus_registers->gmbus1 = gmbus1;
    intel_graphics_device_gmbus_wait_hardware_stop(graphics_device);
  }

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

  {
    Intel_GMBUS1 gmbus1 = graphics_device->gmbus_registers->gmbus1;
    gmbus1.slave_direction = INTEL_GMBUS1_SLAVE_DIRECTION_READ;
    gmbus1.slave_address = address;
    gmbus1.total_byte_count = size;
    gmbus1.bus_cycle_select = INTEL_GMBUS1_BUS_CYCLE_SELECT_WAIT;
    gmbus1.software_ready = 1;
    graphics_device->gmbus_registers->gmbus1 = gmbus1;  
  }
  
  volatile uint32_t *reg = (volatile uint32_t *)&graphics_device->gmbus_registers->gmbus3;

  uint8_t *byte_data = (uint8_t *)buffer;
  size_t bytes_read = 0;
  while (bytes_read < size) {
    intel_graphics_device_gmbus_wait_hardware_ready(graphics_device);
    volatile uint32_t value = *reg;
    for (size_t i = 0; i < 4; i++) {
      if (bytes_read >= size) break;
      byte_data[bytes_read] = (value >> (i * 8)) & 0xFF;
      bytes_read += 1;
    }
  }

  intel_graphics_device_gmbus_wait_hardware_complete(graphics_device);

  {
    Intel_GMBUS1 gmbus1 = graphics_device->gmbus_registers->gmbus1;
    gmbus1.bus_cycle_select = INTEL_GMBUS1_BUS_CYCLE_SELECT_STOP;
    graphics_device->gmbus_registers->gmbus1 = gmbus1;
  }

  return Error_Code_NONE;
}

#if 0
  disableDac();
  disablePipe();
  disableDpll();
  relinquishVga();

  programDpll(params, multiplier);
  dumpDpll();

  programPipe(mode);
  dumpPipe();
  enablePlane(&fb);
enableDac();
#endif

Error_Code intel_graphics_device_display_mode_set(Intel_Graphics_Device *graphics_device, Display_Mode *display_mode) {
  intel_graphics_device_dac_disable(graphics_device);
  intel_graphics_device_pipe_disable(graphics_device);
  intel_graphics_device_dac_enable(graphics_device);
  return Error_Code_NONE;
}


void intel_graphics_device_pipe_disable(Intel_Graphics_Device *graphics_device) {
  uint32_t value = mmio_register_read32(graphics_device->mmio_registers_address, INTEL_MMIO_REGISTER_PIPE_A_CONFIG);
  value &= ~INTEL_PIPE_ENABLE;
  mmio_register_write32(graphics_device->mmio_registers_address, INTEL_MMIO_REGISTER_PIPE_A_CONFIG, value);
  while (value & INTEL_PIPE_STATE) {
    value = mmio_register_read32(graphics_device->mmio_registers_address, INTEL_MMIO_REGISTER_PIPE_A_CONFIG);
  }
}

void intel_graphics_device_dac_enable(Intel_Graphics_Device *graphics_device) {
  uint32_t value = mmio_register_read32(graphics_device->mmio_registers_address, INTEL_MMIO_REGISTER_DAC);
  value |= INTEL_DAC_ENABLE;
  mmio_register_write32(graphics_device->mmio_registers_address, INTEL_MMIO_REGISTER_DAC, value);
}

void intel_graphics_device_dac_disable(Intel_Graphics_Device *graphics_device) {
  uint32_t value = mmio_register_read32(graphics_device->mmio_registers_address, INTEL_MMIO_REGISTER_DAC);
  value &= ~INTEL_DAC_ENABLE;
  mmio_register_write32(graphics_device->mmio_registers_address, INTEL_MMIO_REGISTER_DAC, value);
}