
void convert_detailed_timing_descriptor_to_display_mode(Detailed_Timing_Description *dtd, Display_Mode *mode) {
  mode->horizontal_resolution = dtd->horizontal_active_pixels_lsb;
  mode->horizontal_resolution |= dtd->horizontal_active_pixels_msb << 8;
  mode->vertical_resolution = dtd->vertical_active_lines_lsb;
  mode->vertical_resolution |= dtd->vertical_active_lines_msb << 8;
}

Error_Code extract_display_mode_information(Extended_Display_Identification *edid, Display_Mode *mode) {
  uint64_t checksum = memory_sum(edid, 127);
  if ((checksum & 0xFF) != edid->checksum) {
    klog_error("EDID computed checksum is not valid!");
    //return Error_Code_INVALID_DATA;  
  }

  if (edid->signature != EDID_SIGNATURE) {
    klog_error("EDID signature is invalid: 0x%X!", edid->signature);
    return Error_Code_INVALID_DATA;
  }

  if (edid->version != 1 || edid->revision != 3) {
    klog_error("Invalid EDID version: %u.%u", (uint32_t)edid->version, (uint32_t)edid->revision);
    return Error_Code_INVALID_DATA;
  }

  convert_detailed_timing_descriptor_to_display_mode(&edid->detailed_timings[0], mode);
  return Error_Code_NONE;
}