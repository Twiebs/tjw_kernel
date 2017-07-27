#define KTASK_MAX_PROCESSESS 32
#define KTASK_MAX_THREADS 64

#define KTASK_INVALID_PID 0xFFFFFFFFFFFFFFFF
#define KTASK_INVALID_TID 0xFFFFFFFFFFFFFFFF

//NOTE(Torin 2016-09-01) The entire tasking system is wrriten to be completly 
//brain dead.  It is a very bad implementation but that is the point

typedef struct {
  uint64_t process_id;
} Process_ID;

typedef struct {
  uint64_t thread_id;
} Thread_ID;

static const Process_ID Process_ID_INVALID = { 0xFFFFFFFFFFFFFFFF };
static const Thread_ID Thread_ID_INVALID = { 0xFFFFFFFFFFFFFFFF };

typedef struct {
  uintptr_t executable_physical_address;
  uintptr_t program_start_virtual_address;
  uintptr_t p2_table_physical;
  bool is_valid;
} Process_Context;

typedef struct {
  uint64_t pid;
  uintptr_t stack_physical_address;
  uintptr_t rsp;
  uintptr_t rip;
  bool is_valid;
} Thread_Context;

//NOTE(Torin 2016-09-01) Quick, simple and dirty task info
//for getting things up and running
typedef struct {
  Process_Context processess[KTASK_MAX_PROCESSESS];
  Thread_Context threads[KTASK_MAX_THREADS];
  Thread_Context *running_threads[SYSTEM_MAX_CPU_COUNT];
} Task_Info;

typedef struct {
  uintptr_t entry_address;
  uintptr_t code_location;
  uint64_t code_offset;
  uint64_t code_size;
  uintptr_t data_location;
  uint64_t data_offset;
  uint64_t data_size; 
} Executable_Load_Info;


uint64_t ktask_create_process(Executable_Load_Info *load_info, Task_Info *task_info);
uint64_t ktask_create_thread(uint64_t pid, uintptr_t rip, Task_Info *task_info);

void ktask_context_switch(uint64_t thread_id, Task_Info *task_info);
void ktask_destroy_process(uint64_t pid, Task_Info *task_info);
void ktask_destroy_thread(Thread_Context *ctx);