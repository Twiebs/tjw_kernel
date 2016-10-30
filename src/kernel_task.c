
#define ELF64_IMPLEMENTATION
#include "elf64.h"

//===============================================================================
//TODO(Torin 2016-09-01) Make this take sizes for the data section
//and executable section of the process that is to be created but for now
//It will simply be hardcoded for elf values

//TODO(Torin 2016-10-27) This model is funadmentaly fucked for executables with
//code size > 4096 which is a very common case!

uint64_t ktask_create_process(uintptr_t executable_physical, Task_Info *task_info){
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


  uintptr_t executable_virtual = kmem_push_temporary_kernel_memory(executable_physical);
  ELF64Header *header = (ELF64Header *)executable_virtual;
  if(header->magicNumber != ELF64_MAGIC_NUMBER){
    klog_error("invalid elf file was provided");
    return KTASK_INVALID_PID;
  }
  process->program_start_virtual_address = header->programEntryOffset;
  kmem_pop_temporary_kernel_memory();

  uintptr_t process_page_table = 0;
  kmem_allocate_physical_pages(&globals.memory_state, 1, &process_page_table);
  Page_Table *pt = (Page_Table *)kmem_push_temporary_kernel_memory(process_page_table);
  pt->entries[2] = executable_physical | PAGE_PRESENT_BIT | PAGE_USER_ACCESS_BIT; 
  kmem_pop_temporary_kernel_memory();

  process->process_p2_table = process_page_table;
  process->is_valid = true;
  return result_pid;
}

//TODO(Torin 2016-09-01) Need a real physical frame allocator

uint64_t ktask_create_thread(uint64_t pid, uintptr_t rip, Task_Info *task_info){
  uint64_t result_tid = KTASK_INVALID_TID;
  Thread_Context *thread = 0;
  for(size_t i = 0; i < KTASK_MAX_THREADS; i++){
    Thread_Context *t = &task_info->threads[i];
    if(t->is_valid == false){
      thread = t;
      result_tid = i;
    }
  }

  if(thread == 0) {
    klog_error("NO THREADS");
    kpanic();
  }

  //TODO(Torin 2016-10-20) Broken Thread Creation!

#if 0
  uintptr_t thread_physical_stack = 0x00C00000;
  uintptr_t thread_virtual_stack = 0x00600000;
  kmem_map_physical_to_virtual_2MB_ext(thread_physical_stack, thread_virtual_stack, PAGE_USER_ACCESS_BIT);
  uintptr_t rsp_begin =  thread_virtual_stack + 0x1FFFFF;

  thread->stack_physical_address = thread_physical_stack;
  thread->rsp = rsp_begin;
  thread->rip = rip;
  thread->is_valid = true;
  thread->pid = pid;
  #endif



  return result_tid;
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