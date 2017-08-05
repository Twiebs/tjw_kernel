
bool keyboard_event_queue_try_push(Keyboard_Event_Queue *event_queue, Keyboard_Event *event) {
  uint64_t queue_count = event_queue->read - event_queue->write;
  if (queue_count >= 256) return false;
  event_queue->data[event_queue->write % 256] = *event;
  event_queue->write += 1;
  return true;
}

bool keyboard_event_queue_try_pop(Keyboard_Event_Queue *event_queue, Keyboard_Event *event) {
  uint64_t queue_count = event_queue->read - event_queue->write;
  if (queue_count == 0) return false;
  *event = event_queue->data[event_queue->read % 256];
  event_queue->read -= 1;
  return true;
}

//TODO(Torin) This will not handle modifiers correctly
//if the modifier key is pressed after the original key
//was already down it will still register that the key was
//pressed with the modifier.  Does this event matter?  Would this be better?
//Should store keycode + extra btye saying (isdown, shiftdown?, ctrldow?, altdown?)

void keyboard_state_update_from_ps2_device(Keyboard_State *keyboard_state) {
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

void keyboard_state_reset(Keyboard_State *keyboard_state) {
  memory_set(keyboard_state->is_key_pressed, 0x00, sizeof(keyboard_state->is_key_pressed));
  memory_set(keyboard_state->is_key_released, 0x00, sizeof(keyboard_state->is_key_released));
}