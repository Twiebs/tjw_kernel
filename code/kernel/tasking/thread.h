
typedef struct {
  uint64_t thread_id;
} Thread_ID;

typedef struct {
  uint64_t pid;
  uintptr_t stack_physical_address;
  uintptr_t rsp;
  uintptr_t rip;
  bool is_valid;
} Thread;