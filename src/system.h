
typedef struct {
  uintptr_t kernel_stack_top;
  Task_State_Segment tss;
  uint8_t *temporary_memory;
} CPU_Info;

typedef struct {
  uintptr_t lapic_physical_address;
  uintptr_t ioapic_physical_address;
  uintptr_t lapic_virtual_address;
  uintptr_t ioapic_virtual_address;

  uint64_t total_cpu_count;
  uint64_t running_cpu_count;
  uint64_t cpu_lapic_ids[32];
  CPU_Info cpu_infos[32];
  Spin_Lock cpu_info_lock;

  //Memory Information and managment
  uintptr_t kernel_end;
  uintptr_t memory_begin;
} System_Info;


uint32_t cpu_get_id();
CPU_Info *cpu_get_info();
uint8_t *cpu_get_temporary_memory();

void silly_breakpoint();

