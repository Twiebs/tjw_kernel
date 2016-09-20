global asm_irq_common_handler
global asm_isr_common_handler
global asm_double_fault_handler
global asm_debug_handler

extern irq_common_handler
extern isr_common_handler

extern g_syscall_procedures

%macro PUSH_ALL_REGISTERS 0
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rbp
  push rsi
  push rdi
  push rdx
  push rcx
  push rbx
  push rax
%endmacro

%macro POP_ALL_REGISTERS 0
  pop rax
  pop rbx
  pop rcx
  pop rdx
  pop rdi
  pop rsi
  pop rbp
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  pop r14
  pop r15
%endmacro

asm_double_fault_handler:
  cli
	mov dword [0xb8000], (0x2000 | 'd')
	mov dword [0xb8004], (0x2000 | 'f')
	mov dword [0xb8008], (0x2000 | 'l')
	hlt
	iretq

asm_debug_handler:
  cli
	mov dword [0xb8000], 'a' 
	mov dword [0xb8004], 'b' 
	mov dword [0xb8008], 'c' 
	hlt
	iretq

global asm_spurious_interrupt_handler
asm_spurious_interrupt_handler:
  iretq

asm_irq_common_handler:
  PUSH_ALL_REGISTERS
  call irq_common_handler 
  POP_ALL_REGISTERS
	add esp, 0x8 ;IRQ number pushed in the macro stub
	sti
  iretq

asm_isr_common_handler:
  PUSH_ALL_REGISTERS
	call isr_common_handler 
  POP_ALL_REGISTERS
	add esp, 0x10 ;ISR error code and ISR Number from marco stub
	sti
  iretq

global asm_syscall_handler
asm_syscall_handler:
  cli
  mov rax, [g_syscall_procedures + (rax * 0x8)]
  call rax 
  sti
  iretq



;============================================================================================

%macro DEFINE_IRQ_HANDLER 1
  [GLOBAL asm_irq%1]
	asm_irq%1:
	  cli
		push %1
		jmp asm_irq_common_handler
%endmacro

%macro DEFINE_ISR_NOERROR 1
  [GLOBAL asm_isr%1]
	asm_isr%1:
	  cli
		push 0
		push %1
		jmp asm_isr_common_handler
%endmacro

%macro DEFINE_ISR_ERROR 1
  [GLOBAL asm_isr%1]
	asm_isr%1:
	  cli
		push %1
		jmp asm_isr_common_handler
%endmacro

DEFINE_ISR_NOERROR 0
DEFINE_ISR_NOERROR 1
DEFINE_ISR_NOERROR 2
DEFINE_ISR_NOERROR 3
DEFINE_ISR_NOERROR 4
DEFINE_ISR_NOERROR 5
DEFINE_ISR_NOERROR 6
DEFINE_ISR_NOERROR 7
DEFINE_ISR_ERROR 8
DEFINE_ISR_NOERROR 9
DEFINE_ISR_ERROR 10
DEFINE_ISR_ERROR 11
DEFINE_ISR_ERROR 12
DEFINE_ISR_ERROR 13
DEFINE_ISR_ERROR 14
DEFINE_ISR_NOERROR 15
DEFINE_ISR_NOERROR 16
DEFINE_ISR_ERROR 17
DEFINE_ISR_NOERROR 18
DEFINE_ISR_NOERROR 19
DEFINE_ISR_NOERROR 20
DEFINE_ISR_NOERROR 21
DEFINE_ISR_NOERROR 22
DEFINE_ISR_NOERROR 23
DEFINE_ISR_NOERROR 24
DEFINE_ISR_NOERROR 25
DEFINE_ISR_NOERROR 26
DEFINE_ISR_NOERROR 27
DEFINE_ISR_NOERROR 28
DEFINE_ISR_NOERROR 29
DEFINE_ISR_NOERROR 30
DEFINE_ISR_NOERROR 31

DEFINE_IRQ_HANDLER 0
DEFINE_IRQ_HANDLER 1
DEFINE_IRQ_HANDLER 128


