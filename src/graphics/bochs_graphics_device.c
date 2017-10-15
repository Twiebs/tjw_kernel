
void bochs_graphics_device_write_port(uint16_t register_index, uint16_t value) {
  write_port_uint16(BOCHS_GRAPHICS_DEVICE_PORT_REGISTER, register_index);
  write_port_uint16(BOCHS_GRAPHICS_DEVICE_PORT_DATA, value);
}

uint16_t bochs_graphics_device_read_port(uint16_t register_index) {
  write_port_uint16(BOCHS_GRAPHICS_DEVICE_PORT_REGISTER, register_index);
  uint16_t result = read_port_uint16(BOCHS_GRAPHICS_DEVICE_PORT_DATA);
  return result;
}

void bochs_graphics_device_swap_buffers(Graphics_Device *graphics_device) {
  Bochs_Graphics_Device *bochs_graphics_device = (Bochs_Graphics_Device *)graphics_device;
  uint16_t new_y_offset = bochs_graphics_device->current_framebuffer == 
    &bochs_graphics_device->framebuffer_a ? 0 : bochs_graphics_device->framebuffer_b.height;
  bochs_graphics_device->current_framebuffer = bochs_graphics_device->current_framebuffer == 
    &bochs_graphics_device->framebuffer_a ? &bochs_graphics_device->framebuffer_b : &bochs_graphics_device->framebuffer_a;
   bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_OFFSET_Y, new_y_offset);
}

void bochs_graphics_device_get_back_buffer(Graphics_Device *graphics_device, Software_Graphics_Buffer *buffer) {
  Bochs_Graphics_Device *bochs_graphics_device = (Bochs_Graphics_Device *)graphics_device;
  *buffer = *bochs_graphics_device->current_framebuffer;
}

Error_Code bochs_graphics_device_set_display_mode(Bochs_Graphics_Device *bochs_graphics_device, Display_Mode *display_mode) {
  uint16_t virtual_display_width = display_mode->horizontal_resolution * 1;
  uint16_t virtual_display_height = display_mode->vertical_resolution * 2;
  if (bochs_graphics_device->max_display_width < virtual_display_width) {
  	klog_error("bochs graphics device does not support display mode width");
  	return Error_Code_HARDWARE_FEATURE_UNAVAILABLE;
  }

  if (bochs_graphics_device->max_display_height < virtual_display_height) {
  	klog_error("bochs_graphics_device does not support display mode height");
  	return Error_Code_HARDWARE_FEATURE_UNAVAILABLE;
  }

  bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_ENABLE, BOCHS_GRAPHICS_DEVICE_DISABLED);
  bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_RESOLUTION_X, display_mode->horizontal_resolution);
  bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_RESOLUTION_Y, display_mode->vertical_resolution);
  bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_VIRTUAL_WIDTH, virtual_display_width);
  bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_VIRTUAL_HEIGHT, virtual_display_height);
  bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_BITS_PER_PIXEL, BOCHS_GRAPHICS_DEVICE_BITS_PER_PIXEL_32);
  bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_OFFSET_X, 0);
  bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_OFFSET_Y, 0);
  bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_ENABLE, BOCHS_GRAPHICS_DEVICE_ENABLED | BOCHS_GRAPHICS_DEVICE_ENABLED_LINEAR_FRAME_BUFFER);

  size_t required_memory = (virtual_display_width * virtual_display_height) * 4;
  required_memory = memory_align(required_memory, 4096);
  size_t required_page_count = required_memory / 4096;
  uintptr_t frambuffer_virtual_address = memory_map_physical_mmio(bochs_graphics_device->framebuffer_memory_physical_address, required_page_count);

  klog_debug("bochs_graphics_device: set display_mode");
  klog_debug("  width(%u), height(%u)", display_mode->horizontal_resolution, display_mode->vertical_resolution);

  bochs_graphics_device->framebuffer_a.width = display_mode->horizontal_resolution;
  bochs_graphics_device->framebuffer_a.height = display_mode->vertical_resolution;
  bochs_graphics_device->framebuffer_a.stride = display_mode->horizontal_resolution * 4;
  bochs_graphics_device->framebuffer_a.data = (uint8_t *)frambuffer_virtual_address;
  bochs_graphics_device->framebuffer_b.width = display_mode->horizontal_resolution;
  bochs_graphics_device->framebuffer_b.height = display_mode->vertical_resolution;
  bochs_graphics_device->framebuffer_b.stride = display_mode->horizontal_resolution * 4;
  bochs_graphics_device->framebuffer_b.data = (uint8_t *)(frambuffer_virtual_address + 
  	(display_mode->horizontal_resolution * display_mode->vertical_resolution * 4));
  bochs_graphics_device->current_framebuffer = &bochs_graphics_device->framebuffer_b;
  return Error_Code_NONE;
}

Error_Code bochs_graphics_device_initialize(PCI_Device *pci_device) {
  Bochs_Graphics_Device *bochs_graphics_device = (Bochs_Graphics_Device *)system_allocate_persistent_miscellaneous_device(sizeof(Bochs_Graphics_Device));
  bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_ENABLE, BOCHS_GRAPHICS_DEVICE_GET_CAPABILITIES);
  bochs_graphics_device->max_display_width = bochs_graphics_device_read_port(BOCHS_GRAPHICS_DEVICE_REGISTER_RESOLUTION_X);
  bochs_graphics_device->max_display_height = bochs_graphics_device_read_port(BOCHS_GRAPHICS_DEVICE_REGISTER_RESOLUTION_Y);
  bochs_graphics_device_write_port(BOCHS_GRAPHICS_DEVICE_REGISTER_ENABLE, BOCHS_GRAPHICS_DEVICE_DISABLED);
  bochs_graphics_device->framebuffer_memory_physical_address = pci_device_get_base_address_0(pci_device);
  klog_debug("initialized bochs_graphics_device");
  klog_debug(" max_display_width: %u", bochs_graphics_device->max_display_width);
  klog_debug(" max_display_height: %u", bochs_graphics_device->max_display_height);
  klog_debug(" frambuffer_physical_address: 0x%X", bochs_graphics_device->framebuffer_memory_physical_address);

  bochs_graphics_device->base.get_back_buffer = bochs_graphics_device_get_back_buffer;
  bochs_graphics_device->base.swap_buffers = bochs_graphics_device_swap_buffers;

  Display_Mode display_mode = {};
  //display_mode.horizontal_resolution = 1792;
  //display_mode.vertical_resolution = 1008;
  display_mode.horizontal_resolution = 1280;
  display_mode.vertical_resolution = 720;
  bochs_graphics_device->base.width = display_mode.horizontal_resolution;
  bochs_graphics_device->base.height = display_mode.vertical_resolution;
  Error_Code result = bochs_graphics_device_set_display_mode(bochs_graphics_device, &display_mode);
  klog_debug("bochs_graphics_device initialized");
  globals.graphics_device = &bochs_graphics_device->base;
  globals.system_info.run_mode = System_Run_Mode_DESKTOP_ENVIROMENT;
  return result;
}