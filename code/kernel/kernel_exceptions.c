//TODO(Torin: 2016-10-15) Test all exceptions
//TODO(Torin 2016-10-16) Divide by zero Exception overflows

//NOTE(Torin: 2016-08-24) Exception names derrived from Table found at http://wiki.osdev.org/Exceptions 
static const char *EXCEPTION_NAMES[] = {
  "Divide-by-zero Error",         //00 0x00
  "Debug",                        //01 0x01
  "Non-maskable Interrupt",       //02 0x02
  "Breakpoint",                   //03 0x03
  "Overflow",                     //04 0x04
  "Bound Range Exceeded",         //05 0x05
  "Invalid Opcode",               //06 0x06
  "Device Not Available",         //07 0x07
  "Double Fault",                 //08 0x08
  "Coprocessor Segment Overun",   //09 0x09
  "Invalid TSS",                  //10 0x0A
  "Segment Not Present",          //11 0x0B
  "Stack-Segment Fault",          //12 0x0C
  "General Protection Fault",     //13 0x0D
  "Page Fault",                   //14 0x0E
  "Reserved",                     //15 0x0F
  "x87 Floating-Point Exception", //16 0x10
  "Alignment Check",              //17 0x11
  "Machine Check",                //18 0x12
  "SIMD Floating-Point Exception",//19 0x13
  "Virtualization Exception",     //20 0x14
  "Reserved",                     //21 0x15
  "Reserved",                     //22 0x16
  "Reserved",                     //23 0x17
  "Reserved",                     //24 0x18
  "Reserved",                     //25 0x19
  "Reserved",                     //26 0x1A
  "Reserved",                     //27 0x1B
  "Reserved",                     //28 0x1C
  "Reserved",                     //29 0x1D
  "Security Exception",           //30 0x1E
  "Reserved",                     //31 0x1F
};


static void
isr_handler_general_protection_fault(Interrupt_Stack_Frame stack_frame) {
/* NOTE(Torin: 2016-08-24) Information obtained at http://wiki.osdev.org/Exceptions#General_Protection_Fault
  Common Reasons GPF Exception Will Occur:
  - Segment error (privilege, type, limit, read/write rights).
  - Executing a privileged instruction while CPL != 0.
  - Writing a 1 in a reserved register field.
  - Referencing or accessing a null-descriptor.
  The saved instruction pointer points to the instruction which caused the exception.
  Error code: The General Protection Fault sets an error code, which is the segment selector index when the exception is segment related. Otherwise, 0. 
*/

  struct Selector_Error_Code {
    union {
      struct {
        uint32_t is_external_to_cpu : 1;
        uint32_t table_index : 2;
        uint32_t selector_index : 13;
        uint32_t reserved : 16;
      };
      uint32_t packed;
    };
  } __attribute__((packed)) error_code;

  static const char *TABLE_NAMES[] = {
    "GDT",
    "IDT",
    "LDT",
    "IDT",
  };

  error_code.packed = stack_frame.error_code;
  if(error_code.packed != 0){
    klog_error("GPF caused by segment error: ");
    klog_error(" external_to_cpu: %s", error_code.is_external_to_cpu ? "true" : "false");
    klog_error(" table_name: %s", TABLE_NAMES[error_code.table_index]);
    klog_error(" selector_index: 0x%X", error_code.selector_index);
    kernel_panic();
  } else {
    //TODO(Torin: 2016-08-24) Need to determine if the exception was caused in usermode or kernelmode
    //And then attempt to recover from the error.  This can happen if usermode code attemps
    //to execute a privleged instruction!
    kernel_panic();
  }
}

static void isr_handler_page_fault(Interrupt_Stack_Frame stack_frame) {
	static const uint64_t CAUSE_PROTECTION_VIOLATION_OR_NOT_PRESENT = (1 << 0);
	static const uint64_t CAUSE_WRITE_OR_READ = (1 << 1);
	static const uint64_t CAUSE_USER_OR_KERNEL = (1 << 2);
	static const uint64_t CAUSE_RESERVED_BIT_SET = (1 << 3);
	static const uint64_t CAUSE_INSTRUCTION_FETCH = (1 << 4);

	uintptr_t faulting_address = 0xFFFFFFFFFFFFFFFFL;
	asm volatile ("movq %%cr2, %0" : "=r"(faulting_address));
  if(faulting_address == 0xFFFFFFFFFFFFFFFFL){
    klog_error("invalid faulting_address");
  }

	uint64_t is_protection_violation_else_not_present = stack_frame.error_code & CAUSE_PROTECTION_VIOLATION_OR_NOT_PRESENT;
	uint64_t is_write_else_read = stack_frame.error_code & CAUSE_WRITE_OR_READ;
	uint64_t is_usermode_else_kernel = stack_frame.error_code & CAUSE_USER_OR_KERNEL;
	uint64_t is_reserved_bit_overwritten = stack_frame.error_code & CAUSE_RESERVED_BIT_SET;
	uint64_t is_instruction_else_data = stack_frame.error_code & CAUSE_INSTRUCTION_FETCH;
	
	klog_error("Page Fault: 0x%X", faulting_address);
  klog_error("page is %s%s", (is_protection_violation_else_not_present ? "present" : "not present"), (is_protection_violation_else_not_present ? ", caused by protection violation" : ""));
  klog_error("caused by %s %s %s", is_usermode_else_kernel ? "ring3" : "ring0", is_write_else_read ? "writing" : "reading", is_instruction_else_data ? "a instruction" : "data");
  klog_error("%s", is_reserved_bit_overwritten ? "a reserved bit was overrwriten" : "reserved bits are fine"),
  kdebug_log_interrupt_stack_frame(&stack_frame);

  //memory_debug_log_virtual_address_info_4KB(faulting_address);

	if(is_usermode_else_kernel == false){
		//This is a serious bug there should never be a page-fault in the kernel
		//TODO(Torin) Need better painic mechanisim that handles this sort of thing manualy
		//and mabye drops back into real mode to go back to old-school vga text buffer and
		//does a blue-screen of death type of deal to insure that the error is reported properly
    kernel_panic();
	} else {
		//TODO(TORIN) This was the userspace application lets kill it
    klog_error("UNHANDLED USERSPACE VIOLATION!!!");
    kernel_panic();
	}
}