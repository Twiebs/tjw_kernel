#define KTASK_MAX_PROCESSESS 32
#define KTASK_MAX_THREADS 64

#define KTASK_INVALID_PID 0xFFFFFFFFFFFFFFFF
#define KTASK_INVALID_TID 0xFFFFFFFFFFFFFFFF

//NOTE(Torin 2016-09-01) The entire tasking system is wrriten to be completly 
//brain dead.  It is a very bad implementation but that is the point

typedef struct {
  uintptr_t physical_address;
  uintptr_t start_address;
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

uint64_t ktask_create_process(uintptr_t elf_executable, Task_Info *task_info);
uint64_t ktask_create_thread(uint64_t pid, uintptr_t rip, Task_Info *task_info);
void ktask_context_switch(uint64_t thread_id, Task_Info *task_info);
void ktask_destroy_process(uint64_t pid, Task_Info *task_info);
void ktask_destroy_thread(Thread_Context *ctx);