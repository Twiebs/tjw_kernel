typedef struct {
  uint8_t keystate[256];
  size_t scancode_event_stack_count;
  uint8_t *scancode_event_stack;
  uint8_t scancode_event_stack0[32];
  uint8_t scancode_event_stack1[32];
} Keyboard_State;

static void irq_handler_keyboard(void);
static void irq_handler_pit(void);
