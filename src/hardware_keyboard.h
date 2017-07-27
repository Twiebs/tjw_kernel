

#define KEYBOARD_SCANCODE1_UP_PRESSED 0x48
#define KEYBOARD_SCANCODE1_DOWN_PRESSED 0x50
#define KEYBOARD_SCANCODE1_BACKSPACE_PRESSED  0x0E
#define KEYBOARD_SCANCODE1_BACKSPACE_RELEASED 0x8E
#define KEYBOARD_SCANCODE1_ENTER_PRESSED 0x1C
#define KEYBOARD_SCANCODE1_LSHIFT 0x2A
#define KEYBOARD_SCANCODE1_RSHIFT 0x36

static const char SCANCODE_TO_LOWERCASE_ACII[] = {
  0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 
  0,  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,  
  0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 
  'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',  0, 0, 0, ' ',  
};

static const char SCANCODE_TO_UPERCASE_ACII[] = {
  0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 
  0,  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,  
  0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,  '|', 
  'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',  0, 0, 0, ' ',  
};

typedef struct {
  uint8_t is_key_down[256];
  uint8_t is_key_pressed[256];
  uint8_t is_key_released[256];
} Keyboard_State;

void update_keyboard_state(Keyboard_State *keyboard_state);
void reset_keyboard_state(Keyboard_State *keyboard_state);