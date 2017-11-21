



#if 0
//TODO(Torin 2016-09-01) There is a high probabilty that the cpu_id
//and thus coresponding thread_id will be known at this point
//This entire procedure can proabably just be inlined into whatever is going
//to end up calling it in the future
void ktask_context_switch(uint64_t thread_id, Task_Info *task_info) {
  extern void asm_enter_usermode(uintptr_t rip, uintptr_t rsp);
  uint32_t cpu_id = cpu_get_id();
  Thread_Context *ctx = &task_info->threads[thread_id];
  //TODO(Torin 2016-09-01) This should be the point of no return and interrupts should be
  //stoped here so that there is no shannaigans before we enter usermode propper
  task_info->running_threads[cpu_id] = ctx;
  asm_enter_usermode((uintptr_t)ctx->rip, ctx->rsp);
}
#endif