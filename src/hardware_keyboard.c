
//TODO(Torin) This will not handle modifiers correctly
//if the modifier key is pressed after the original key
//was already down it will still register that the key was
//pressed with the modifier.  Does this event matter?  Would this be better?
//Should store keycode + extra btye saying (isdown, shiftdown?, ctrldow?, altdown?)

void update_keyboard_state(Keyboard_State *keyboard_state) {
  static const uint16_t KEYBOARD_DATA_PORT   = 0x0060;
  static const uint16_t KEYBOARD_KEY_PENDING_PORT = 0x0064;
  uint8_t is_key_pending = read_port_uint8(KEYBOARD_KEY_PENDING_PORT);
  if (is_key_pending & 0x01) { //Keyboard has data ready
    uint8_t scancode = read_port_uint8(KEYBOARD_DATA_PORT);
    if (scancode > 0x80) {  //NOTE(Torin)key release event 
      uint8_t base_scancode = scancode - 0x80;
      keyboard_state->is_key_released[base_scancode] = 1;
      keyboard_state->is_key_down[base_scancode] = 0;
    } else {
      keyboard_state->is_key_pressed[scancode] = 1;
      keyboard_state->is_key_down[scancode] = 1;
    }
  }
}

void reset_keyboard_state(Keyboard_State *keyboard_state) {
  memset(keyboard_state->is_key_pressed, 0x00, sizeof(keyboard_state->is_key_pressed));
  memset(keyboard_state->is_key_released, 0x00, sizeof(keyboard_state->is_key_released));
}