
void desktop_enviroment_initialize(Desktop_Enviroment *desktop_enviroment, Graphics_Device *graphics_device) {
  desktop_enviroment->graphics_device = graphics_device;
  software_graphics_buffer_create(&desktop_enviroment->overlay_buffer, graphics_device->width, graphics_device->height);
  klog_debug("initialized desktop enviroment");
}

void desktop_enviroment_draw_desktop(Desktop_Enviroment *desktop_enviroment) {
  Color_RGB_U8 desktop_background_color = { 44, 62, 80 };
  Software_Graphics_Buffer current_back_buffer = {};
  graphics_device_get_back_buffer(desktop_enviroment->graphics_device, &current_back_buffer);
  software_graphics_buffer_draw_rectangle_solid(&current_back_buffer, 0, 0,
  	desktop_enviroment->overlay_buffer.width, desktop_enviroment->overlay_buffer.height, desktop_background_color);
}

void desktop_enviroment_draw_overlay(Desktop_Enviroment *desktop_enviroment) {
  Color_RGB_U8 overlay_background_color = { 20, 20, 20 };
  software_graphics_buffer_draw_rectangle_solid(&desktop_enviroment->overlay_buffer, 0, 0, 
  	desktop_enviroment->overlay_buffer.width, desktop_enviroment->overlay_buffer.height, overlay_background_color);
  software_graphics_draw_cstring(&desktop_enviroment->overlay_buffer, &INCONSOLATA16, "This is a string.", 25, 25);
}

void desktop_enviroment_draw(Desktop_Enviroment *desktop_enviroment) {
  desktop_enviroment_draw_desktop(desktop_enviroment);
  if (desktop_enviroment->is_overlay_active) {
  	desktop_enviroment_draw_overlay(desktop_enviroment);
  	Software_Graphics_Buffer current_back_buffer = {};
  	graphics_device_get_back_buffer(desktop_enviroment->graphics_device, &current_back_buffer);
	software_graphics_buffer_overlay_buffers(&current_back_buffer, &desktop_enviroment->overlay_buffer, 200);
  }

  graphics_device_swap_buffers(desktop_enviroment->graphics_device);
}

void desktop_enviroment_process_input(Desktop_Enviroment *desktop_enviroment, Keyboard_State *keyboard_state) {
  if (keyboard_state->is_key_pressed[KEYBOARD_SCANCODE1_UP_PRESSED]) {
  	desktop_enviroment->is_overlay_active = !desktop_enviroment->is_overlay_active;
  }
}

void desktop_enviroment_update(Desktop_Enviroment *desktop_enviroment) {
  keyboard_state_update(&globals.keyboard);
  desktop_enviroment_process_input(desktop_enviroment, &globals.keyboard);
  keyboard_state_reset(&globals.keyboard);
  desktop_enviroment_draw(desktop_enviroment);
}