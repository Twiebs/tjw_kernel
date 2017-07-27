
#define ELF64_IMPLEMENTATION
#include "elf64.h"

static inline
void ktask_log_process_info(Process_Context *p){
  klog_debug("executable_physical_address: 0x%X", p->executable_physical_address);
  klog_debug("program_start_virtual_address: 0x%X", p->program_start_virtual_address);
  klog_debug("is_valid: %u", (uint32_t)p->is_valid);
}

#define exit_if(condition, message) if((condition)){klog_error(message);return false;}

static inline
bool extract_elf_executable_load_info(uintptr_t executable_virtual, Executable_Load_Info *info){
  ELF64_Header *elf_header = (ELF64_Header *)executable_virtual;
  exit_if(elf_header->magic_number != ELF64_MAGIC_NUMBER, "ELF executable magic number mismatch");
  exit_if(elf_header->elf_class != ELF_CLASS_64, "ELF executable must be 64bit format");
  exit_if(elf_header->data_encoding != ELF_DATA_ENCODING_LITTLE_ENDIAN, "ELF Executable must be endcoded Little Endian");
  exit_if(elf_header->abi_type != ELF_ABI_TYPE_SYSTEMV, "ELF executable must use SystemV ABI");
  
  info->entry_address = elf_header->entry_address;

  ELF64_Program_Header *program_header = (ELF64_Program_Header *)(executable_virtual + elf_header->program_header_offset);
  for(size_t i = 0; i < elf_header->program_header_entry_count; i++){
    if(program_header->segment_type == ELF_Segment_Type_LOAD){
      if(info->code_location != 0x00){
        klog_error("unhandled multipule load case");
        return false;
      }

      info->code_location = program_header->virtual_address;
      info->code_offset = program_header->offset_in_file;
    }

    program_header = (ELF64_Program_Header *)(((uintptr_t)program_header) + elf_header->program_header_entry_size);
  }

  return true;
}

//NOTE(Torin 2016-11-28) Need p4 table for each cpu core

bool ktask_run_program(const char *program_path, size_t path_length){
  File_Handle handle = {};
  if(fs_obtain_file_handle(program_path, path_length, &handle) == 0){
    klog_error("failed to open program executable: %.*s", path_length, program_path);
    return false;
  }

  //TODO(Torin) Make this smater for potentional overflows
  uint64_t required_memory = ((handle.file_size + 0xFFF) & ~0xFFF);
  uint64_t required_pages = required_memory / 4096;
  if(required_pages > 4096) {
    klog_error("Executable is too large for current physical page allocation method");
    return false;
  }

  uintptr_t physical_pages[required_pages];
  memset(physical_pages, 0x00, sizeof(physical_pages));
  if(kmem_allocate_physical_pages(&globals.memory_state, required_pages, physical_pages) == 0){
    klog_error("System is out of physical memory.  Not enough RAM to run program: %.*s", path_length, program_path);
    return false;
  }

  if(fs_read_file(&handle, 0, handle.file_size, physical_pages) == 0){
    klog_error("failed to read file");
    return false;
  }

  Executable_Load_Info load_info = {};
  uintptr_t executable_virtual = kmem_push_temporary_kernel_memory(physical_pages[0]);
  extract_elf_executable_load_info(executable_virtual, &load_info);
  kmem_pop_temporary_kernel_memory();
  uint64_t pid = ktask_create_process(&load_info, &globals.task_info);

  if (pid == KTASK_INVALID_PID){
    klog_error("failed to create process");
    return false;
  }


  Process_Context *p = &globals.task_info.processess[pid];
  ktask_log_process_info(p);
  
  //uint64_t thread_id = ktask_create_thread(pid, p->start_address, &globals.task_info);
  //ktask_context_switch(thread_id, &globals.task_info);
  return true;
}


uint64_t ktask_create_process(Executable_Load_Info *load_info, Task_Info *task_info){
  uint64_t result_pid = KTASK_INVALID_PID;
  Process_Context *process = 0;

  for(size_t i = 0; i < KTASK_MAX_PROCESSESS; i++){
    Process_Context *p = &task_info->processess[i];
    if(p->is_valid == false) {
      process = p;
      result_pid = i;
      break;
    }
  }

  if(process == 0) return result_pid;


  process->program_start_virtual_address = load_info->entry_address;

  uintptr_t process_page_table_physical = 0;
  kmem_allocate_physical_pages(&globals.memory_state, 1, &process_page_table_physical);
  Page_Table *pt = (Page_Table *)kmem_push_temporary_kernel_memory(process_page_table_physical);
  pt->entries[2] = load_info->code_location | PAGE_PRESENT_BIT | PAGE_USER_ACCESS_BIT; 
  kmem_pop_temporary_kernel_memory();

  process->p2_table_physical = process_page_table_physical;
  process->is_valid = true;
  return result_pid;
}

uint64_t ktask_create_thread(uint64_t pid, uintptr_t rip, Task_Info *task_info){
  uint64_t result_thread_id = KTASK_INVALID_TID;
  Thread_Context *thread = 0;
  for(size_t i = 0; i < KTASK_MAX_THREADS; i++){
    Thread_Context *t = &task_info->threads[i];
    if(t->is_valid == false){
      thread = t;
      result_thread_id = i;
    }
  }

  if(thread == 0) {
    klog_error("NO THREADS");
    kpanic();
  }

#if 0
  Process_Context *p = &globals.task_info.processess[pid];
  uintptr_t thread_data_stack_physical_pages[512] = {};
  kmem_allocate_physical_pages(&globals.memory_state, 512, thread_data_stack_physical_pages);
  Page_Table *pt = (Page_Table *)kmem_push_temporary_kernel_memory(p->p2_table_physical);
  pt->entries[3] = thread_data_stack_physical_pages  

  uintptr_t rsp_begin =  thread_virtual_stack + 0x1FFFFF;

  thread->stack_physical_address = thread_physical_stack;
  thread->rsp = rsp_begin;
  thread->rip = rip;
  thread->is_valid = true;
  thread->pid = pid;
  #endif



  return result_thread_id;
}

//TODO(Torin 2016-09-01) There is a high probabilty that the cpu_id
//and thus coresponding thread_id will be known at this point
//This entire procedure can proabably just be inlined into whatever is going
//to end up calling it in the future
void ktask_context_switch(uint64_t thread_id, Task_Info *task_info){
  extern void asm_enter_usermode(uintptr_t rip, uintptr_t rsp);
  uint32_t cpu_id = get_cpu_id();
  Thread_Context *ctx = &task_info->threads[thread_id];
  //TODO(Torin 2016-09-01) This should be the point of no return and interrupts should be
  //stoped here so that there is no shannaigans before we enter usermode propper
  task_info->running_threads[cpu_id] = ctx;
  asm_enter_usermode((uintptr_t)ctx->rip, ctx->rsp);
}

void ktask_destroy_process(uint64_t pid, Task_Info *task_info){
  Process_Context *p = &task_info->processess[pid];
  p->is_valid = false;
  //TODO(Torin)
  // - Deallocate physical frames
  // - free allocated memory

  for(size_t i = 0; i < KTASK_MAX_THREADS; i++){
    Thread_Context *t = &task_info->threads[i];
    if(t->pid == pid) {
      ktask_destroy_thread(t);
    }
  }
}

void ktask_destroy_thread(Thread_Context *ctx){
  ctx->is_valid = false;
  //TODO(Torin)
  // Free the stack!
}

//===============================================================================

#if 0
static inline
int ktask_load_elf_executable(uintptr_t elf_executable){
    
#if 0
  klog_debug("program entry offset: 0x%X", header->programEntryOffset);
  for(size_t i = 0; i < header->programHeaderEntryCount; i++){
    ELF64ProgramHeader *program_header = (ELF64ProgramHeader *)(elf_executable + header->programHeaderOffset + (i * header->programHeaderEntrySize));
    klog_debug("program header entry:");
    klog_debug(" segment virtual memory location: 0x%X", program_header->virtual_address);
    klog_debug(" segment file offset: 0x%X", program_header->offset_in_file);
    klog_debug(" segment file size: 0x%X", program_header->segment_file_size);
    klog_debug(" segment memory size: 0x%X", program_header->segment_memory_size);
  }
  #endif

  return header->programEntryOffset;
}
#endif