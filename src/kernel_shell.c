


#define COMMAND_UNKNOWN_COMMAND 1
#define COMMAND_INVALID_ARG_COUNT 2

#define KShell_Command_Metalist \
_(help, kshell_help) \
_(hardware_info, kshell_hardware_info)\
_(time, kshell_unimplemented_command)\
_(ioapic_irq_map, kshell_ioapic_irq_map)

typedef enum {
  KShell_Command_Invalid,
  #define _(name, proc) KShell_Command_##name,
  KShell_Command_Metalist
  #undef _
  KShell_Command_Count
} KShell_Command;

static const char *KShell_Command_Name[] = {
#define _(name, proc) #name,
KShell_Command_Metalist
#undef _
};

static void kshell_help(const char *text, size_t length);
static void kshell_hardware_info(const char *text, size_t length);
static void kshell_unimplemented_command(const char *text, size_t length);
static void kshell_ioapic_irq_map(const char *text, size_t length);

typedef void(*KShell_Command_Proc)(const char *, size_t);
static const KShell_Command_Proc KShell_Command_Handler[] = {
#define _(name, proc) proc,
KShell_Command_Metalist
#undef _
};

static void
kshell_help(const char *input, size_t length){
  klog_info("kshell command list:");
  #define _(name, proc) \
  klog_info("  %s", #name);
  KShell_Command_Metalist
  #undef _
}

static void
kshell_ioapic_irq_map(const char *text, size_t length){
  ioapic_log_irq_map(globals.system_info.ioapic_virtual_address);
}

static void
kshell_hardware_info(const char *text, size_t length) {

}

static void
kshell_unimplemented_command(const char *text, size_t length){
  klog_error("%s is an unimplemented command!", text);
}

void kshell_process_command(const char *input, size_t length){
  #define _(name, proc) \
  if(string_matches_string(#name, sizeof(#name)-1, input)){\
    proc(input, length);\
    return;\
  }
  KShell_Command_Metalist
  #undef _

  klog_error("%s is not a shell command", input);
}

#if 0
static int 
kshell_is_command_signature_valid(const char *input, size_t input_len,
		const char *command_name, size_t command_length,
		uint32_t expected_arg_count, uint32_t actual_arg_count) 
{
	if (strings_match(input, input_len, command_name, command_length)) {
		if (expected_arg_count != actual_arg_count) {
			klog_error("%s expected %u arguments, only %u were provided", command_name, expected_arg_count, actual_arg_count);
			return COMMAND_INVALID_ARG_COUNT;
		} else {
			return 0;
		}
	}
	return COMMAND_UNKNOWN_COMMAND;
}
#endif

#if 0
static bool 
kshell_process_command(const char *command) {
  const char *procedure_name = command;
  size_t arg_count = 0;
  const char *args[6];
  size_t arg_lengths[6];

  const char *current = command;
  while (is_char_alpha(*current)) current++;
  size_t procedure_name_length = current - procedure_name;
  while (*current == ' ') {
    current++; 
    args[arg_count] = current;
    while (*current != ' ' && *current != 0) current++;
    arg_lengths[arg_count] = current - args[arg_count];
    arg_count++;
  }
  
#define command_hook(name, argc)  \
  else if (!(command_validity_status = is_command_signature_valid(procedure_name, procedure_name_length,  \
          name, LITERAL_STRLEN(name), argc, arg_count)))

  bool command_was_handled = false;
  int command_validity_status = 0;

  if (0) {}
  command_hook("reboot", 0) { kernel_reboot(); }

  command_hook("ls", 0) {
    for (uint32_t i = 0; i < _fs.kfs->node_count; i++) {
      KFS_Node *node = &_fs.kfs_nodes[i];
      klog_info("filename: %s, filesize: %u", node->name, node->size);
    }
  }

  command_hook("print", 1) {
    KFS_Node *node = kfs_find_file_with_name(args[0], arg_lengths[0]);
    if (node == 0) {
      klog_info("file %.*s could not be found", arg_lengths[0], args[0]);
    } else {
      const uint8_t *node_data = _fs.base_data + node->offset;
      klog_info("[%s]: %.*s", node->name, node->size, node_data);
    }
  }



  if (command_validity_status == COMMAND_UNKNOWN_COMMAND) {
    klog_info("unknown command '%s'", command);
    return false;
  } else if (command_validity_status == COMMAND_INVALID_ARG_COUNT) {
    return false;
  }

  return true;
}
#endif