global asm_irq_common_handler
global asm_isr_common_handler
global asm_double_fault_handler
global asm_debug_handler

extern irq_common_handler
extern isr_common_handler

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

asm_irq_common_handler:
	push rax
	push rbx
	push rcx
  
	call irq_common_handler 
 	
	pop rcx
	pop rbx
	pop rax
	add esp, 0x8 ;IRQ number pushed in the macro stub
	sti
  iretq

asm_isr_common_handler:
  push rax
	push rbx
	push rcx

	call isr_common_handler 
	
	pop rcx
	pop rbx
	pop rax

	add esp, 0x10 ;ISR error code and ISR Number from marco stub
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


