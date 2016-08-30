typedef struct {
  uint8_t keystate[256];
  size_t scancode_event_stack_count;
  uint8_t *scancode_event_stack;
  uint8_t scancode_event_stack0[32];
  uint8_t scancode_event_stack1[32];
} Keyboard_State;

//NOTE(Torin: 2016-08-24) An error_code is always pushed
//on to the stack in our execption and interrupt handling model
typedef struct {
  //TODO(Torin) These should be treated seperatly
 	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax; 

  uint64_t interrupt_number;
  uint64_t error_code;
  uint64_t rip;
  uint64_t cs;  
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} Interrupt_Stack_Frame;

typedef struct {
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} Interrupt_Stack_Frame_No_Error;

static void irq_handler_keyboard(void);
static void irq_handler_pit(void);