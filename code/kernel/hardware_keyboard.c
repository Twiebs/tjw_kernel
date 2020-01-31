
bool keyboard_event_queue_try_push(Keyboard_Event_Queue *event_queue, Keyboard_Event *event) {
  uint64_t queue_count = event_queue->write - event_queue->read;
  if (queue_count >= 256) return false;
  event_queue->data[event_queue->write % 256] = *event;
  event_queue->write += 1;
  return true;
}

bool keyboard_event_queue_try_pop(Keyboard_Event_Queue *event_queue, Keyboard_Event *event) {
  uint64_t queue_count = event_queue->write - event_queue->read;
  if (queue_count == 0) return false;
  *event = event_queue->data[event_queue->read % 256];
  event_queue->read += 1;
  return true;
}

//TODO(Torin 2017-08-13) This procedure is very slow and hacky
//because im too lazy to make it constant time
Keyboard_Keycode keyboard_scancode_to_keycode(int scancode, bool *is_key_released) {
  static const Keyboard_Scancode_Info KEYBOARD_SCANCODE1_INFO_ARRAY[] = {
    { false, 0x3B, Keyboard_Keycode_F1, },
    { false, 0x02, Keyboard_Keycode_1, },
    { false, 0x2A, Keyboard_Keycode_LEFT_SHIFT, },
    { true, 0xAA, Keyboard_Keycode_LEFT_SHIFT, },
  };

  
  size_t count = ARRAY_COUNT(KEYBOARD_SCANCODE1_INFO_ARRAY);
  for (size_t i = 0; i < count; i++) {
    const Keyboard_Scancode_Info *info = &KEYBOARD_SCANCODE1_INFO_ARRAY[i];
    if (info->scancode == scancode) {
      *is_key_released = info->released;
      return info->keycode;
    }

  }

  *is_key_released = scancode > 0x80;
  return scancode;
}

void keyboard_state_add_scancodes_from_ps2_device(Keyboard_State *keyboard_state) {
  static const uint16_t KEYBOARD_DATA_PORT   = 0x0060;
  static const uint16_t KEYBOARD_KEY_PENDING_PORT = 0x0064;
  uint8_t is_key_pending = read_port_uint8(KEYBOARD_KEY_PENDING_PORT);
  if (is_key_pending & 0x01) { //Keyboard has data ready
    uint8_t scancode = read_port_uint8(KEYBOARD_DATA_PORT);
    Keyboard_Event event = { scancode };
    keyboard_event_queue_try_push(&keyboard_state->event_queue, &event);
  }
}

void keyboard_state_update(Keyboard_State *keyboard_state) {
  Keyboard_Event event;
  while (keyboard_event_queue_try_pop(&keyboard_state->event_queue, &event)) {
    bool is_key_released = false;
    Keyboard_Keycode keycode = keyboard_scancode_to_keycode(event.scancode, &is_key_released);
    if (is_key_released) {  //NOTE(Torin)key release event 
      keyboard_state->is_key_released[keycode] = true;
      keyboard_state->is_key_down[keycode] = false;
    } else {
      keyboard_state->is_key_pressed[keycode] = true;
      keyboard_state->is_key_down[keycode] = true;
    }
  }
}






void keyboard_state_reset(Keyboard_State *keyboard_state) {
  memory_set(keyboard_state->is_key_pressed, 0x00, sizeof(keyboard_state->is_key_pressed));
  memory_set(keyboard_state->is_key_released, 0x00, sizeof(keyboard_state->is_key_released));
}