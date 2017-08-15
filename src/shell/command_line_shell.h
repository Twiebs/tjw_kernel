
typedef struct {
  size_t length;
  char *text;
} Shell_Command_Parameter;

typedef struct {
  uint64_t parameter_count;
  Shell_Command_Parameter parameters[16];
} Shell_Command_Parameter_Info;

typedef void(*Shell_Command_Procedure)(Shell_Command_Parameter_Info *parameter_info);

typedef struct {
  char name[128];
  uint64_t name_count;
  uint64_t parameter_count;
  Shell_Command_Procedure procedure;
} Shell_Command;

typedef struct {
  bool     requires_redraw;
  uint64_t line_offset;
  uint64_t character_number;
  uint64_t characters_per_line;
  uint64_t lines_per_screen;

  char     input_buffer[256];
  uint64_t input_buffer_count;
  char     current_directory[1024];
  size_t   current_directory_count;

  Shell_Command commands[64];
  uint64_t command_count;
  Shell_Command_Parameter_Info parameter_info;
  
} Command_Line_Shell;

static const char * LOG_CATEGORY_TAGS[] = {
  "[Default] ",
  "[Debug0] ",
  "[Memory] ",
  "[VFS] ",
};

void shell_initialize(Command_Line_Shell *shell);
void shell_update(Command_Line_Shell *shell);
void shell_draw_if_required(Command_Line_Shell *shell, Circular_Log *log);
void shell_process_keyboard_input(Command_Line_Shell *shell, Keyboard_State *keyboard);
void shell_add_input_character(Command_Line_Shell *shell, const char c);
void shell_remove_last_input_character(Command_Line_Shell *shell);
void shell_command_register(Command_Line_Shell *shell, const char *name, uint64_t parameter_count, Shell_Command_Procedure procedure);
