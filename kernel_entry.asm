bits 32
section .text
  align 4
  dd 0x1BADB002
  dd 0x00
  dd - (0x1BADB002 + 0x00)

global start

extern kernel_entry


extern irq_common_handler
extern irq_default_handler 
extern irq_handler_keyboard
extern irq_handler_breakpoint


global read_port
global write_port
global load_idt

global asm_irq_default_handler
global asm_irq_handler_keyboard
global asm_irq_breakpoint
global asm_irq_debug


read_port:
  mov edx, [esp + 4]
  in al, dx
  ret

write_port:
  mov edx, [esp + 4]
  mov al,  [esp + 8]
  out dx,  al
  ret

asm_irq_common_handler:
  pusha

  mov ax, ds
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

  call irq_common_handler 
  
	pop ebx
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx

	popa
	sti
	add esp, 4
  iretd

[GLOBAL asm_irq_unhandled_stub]
asm_irq_unhandled_stub:
  cli
	push 0xFFFF
	jmp asm_irq_common_handler


%macro DEFINE_IRQ_HANDLER 1
  [GLOBAL asm_irq%1]
	asm_irq%1:
	  cli
		push %1
		jmp asm_irq_common_handler
%endmacro

DEFINE_IRQ_HANDLER 0
DEFINE_IRQ_HANDLER 1

%if 0
asm_irq_handler_keyboard:
  cli
  pusha

  mov ax, ds
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

  call irq_handler_keyboard
  
	pop ebx
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx

	popa
	sti
  iretd
%endif

start:
  cli
  mov esp, stack_space
  call kernel_entry

section .bss
resb 8192
stack_space:
