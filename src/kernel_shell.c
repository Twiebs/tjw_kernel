


#define COMMAND_UNKNOWN_COMMAND 1
#define COMMAND_INVALID_ARG_COUNT 2

#define KShell_Command_Metalist \
_(help, kshell_help) \
_(hardware_info, kshell_hardware_info)\
_(time, kshell_unimplemented_command)\
_(ioapic_irq_map, kshell_ioapic_irq_map)\
_(run, kshell_run)

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
static void kshell_run(const char *text, size_t length);

typedef void(*KShell_Command_Proc)(const char *, size_t);
static const KShell_Command_Proc KShell_Command_Handler[] = {
#define _(name, proc) proc,
KShell_Command_Metalist
#undef _
};

#define string_and_length(string) string, (sizeof(string) - 1)

static void
kshell_help(const char *input, size_t length){
  klog_info("kshell command list:");
  #define _(name, proc) \
  klog_info("  %s", #name);
  KShell_Command_Metalist
  #undef _
}

static void
kshell_run(const char *text, size_t length){
  uintptr_t executable_physical_page;
  kmem_allocate_physical_pages(&globals.memory_state, 1, &executable_physical_page);
  if(ext2fs_read_file_to_physical(string_and_length("/programs/test_program"), executable_physical_page)){
    klog_error("failed to read program executable");
    return;
  }


  uint64_t pid = ktask_create_process(executable_physical_page, &globals.task_info);
  if(pid == KTASK_INVALID_PID) {
    klog_error("failed to create process");
    return;
  }

  Process_Context *p = &globals.task_info.processess[pid];
  //uint64_t thread_id = ktask_create_thread(pid, p->start_address, &globals.task_info);
  //ktask_context_switch(thread_id, &globals.task_info);
}

static void
kshell_ioapic_irq_map(const char *text, size_t length){
  kdebug_ioapic_log_irq_map(globals.system_info.ioapic_virtual_address);
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
