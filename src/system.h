
typedef struct {
  uintptr_t kernel_stack_top;
  Task_State_Segment tss;
  uint8_t *temporary_memory;
} CPU_Info;

//NOTE(Torin) Many of the arrays in this structure are staticly allocated
//fixed sized, and persistant. This is for testing purposes and will not be
//the final implementation.
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

  //PCI Device Managment
  PCI_Device pci_devices[32];
  uint64_t pci_device_count;
  //TODO(Torin 2017-08-20) Unsure of how to do this nicely.  For now
  //we store a array of driver structures which contain information
  //about how to initalize the device and extract description information
  PCI_Device_Driver pci_device_drivers[16];
  uint64_t pci_device_driver_count;

  //Memory Information and managment
  uintptr_t kernel_end;
  uintptr_t memory_begin;
} System_Info;




uint32_t cpu_get_id();
CPU_Info *cpu_get_info();
uint8_t *cpu_get_temporary_memory();

void silly_breakpoint();

