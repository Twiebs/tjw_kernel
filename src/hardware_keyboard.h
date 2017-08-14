#define KEYBOARD_SCANCODE1_UP_PRESSED 0x48
#define KEYBOARD_SCANCODE1_DOWN_PRESSED 0x50
#define KEYBOARD_SCANCODE1_RIGHT_PRESSED 0x4D
#define KEYBOARD_SCANCODE1_LEFT_PRESSED 0x4B
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


typedef enum {
  Keyboard_Keycode_INVALID,
  Keyboard_Keycode_F1 = 0x3B, //HACK
  Keyboard_Keycode_1 = 0x02, //HACK
} Keyboard_Keycode;

typedef struct {
  bool released;
  int scancode;
  Keyboard_Keycode keycode;
} Keyboard_Scancode_Info;

typedef enum {
  Keyboard_Key_State_PRESSED,
  Keyboard_Key_State_RELEASED,
} Keyboard_Key_State;

typedef struct {
  uint8_t scancode;
} Keyboard_Event;

typedef struct {
  Keyboard_Event data[256];
  uint64_t read;
  uint64_t write;
} Keyboard_Event_Queue;

typedef struct {
  bool is_key_down[256];
  bool is_key_pressed[256];
  bool is_key_released[256];
  Keyboard_Event_Queue event_queue;
} Keyboard_State;


bool keyboard_event_queue_try_push(Keyboard_Event_Queue *event_queue, Keyboard_Event *event);
bool keyboard_event_queue_try_pop(Keyboard_Event_Queue *event_queue, Keyboard_Event *event);
void keyboard_state_add_scancodes_from_ps2_device(Keyboard_State *keyboard_state);
Keyboard_Keycode keyboard_scancode_to_keycode(int scancode, bool *is_key_released);

void keyboard_state_update(Keyboard_State *keyboard_state);
void keyboard_state_reset(Keyboard_State *keyboard_state);