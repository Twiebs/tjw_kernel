
void debug_log_display_mode(Display_Mode *mode) {
  klog_debug("Display Mode");
  klog_debug("  horizontal_resolution: %u", mode->horizontal_resolution);
  klog_debug("  vertical_resolution: %u", mode->vertical_resolution);
}