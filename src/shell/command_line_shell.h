
typedef void(*Command_Line_Shell_Command_Procedure)(const char *, size_t);

typedef struct {
  char name[128];
  Command_Line_Shell_Command_Procedure procedure;
} Command_Line_Shell_Command;

typedef struct {
  uint64_t line_number;
  uint64_t character_number;
  char input_buffer[256]
  Command_Line_Shell_Command commands[64];
  uint64_t command_count;
} Command_Line_Shell;

void register_shell_command(const char *name, Command_Line_Shell_Command_Procedure *procedure) {
  
}