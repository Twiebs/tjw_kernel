bits 32


MBOOT_PAGE_ALIGN    equ 1<<0    ; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1    ; Provide your kernel with memory info
MBOOT_HEADER_MAGIC  equ 0x1BADB002 ; Multiboot Magic value
; NOTE: We do not use MBOOT_AOUT_KLUDGE. It means that GRUB does not
; pass us a symbol table.
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)


[BITS 32]                       ; All instructions should be 32-bit.

[GLOBAL mboot]                  ; Make 'mboot' accessible from C.
[EXTERN code]                   ; Start of the '.text' section.
[EXTERN bss]                    ; Start of the .bss section.
[EXTERN end]                    ; End of the last loadable section.

mboot:
  dd  MBOOT_HEADER_MAGIC        ; GRUB will search for this value on each
                                ; 4-byte boundary in your kernel file
  dd  MBOOT_HEADER_FLAGS        ; How GRUB should load your file / settings
  dd  MBOOT_CHECKSUM            ; To ensure that the above values are correct
   
  dd  mboot                     ; Location of this descriptor
  dd  code                      ; Start of kernel '.text' (code) section.
  dd  bss                       ; End of kernel '.data' section.
  dd  end												; End of kernel.
  dd  start                     ; Kernel entry point (initial EIP).
section .text
  align 4
  dd 0x1BADB002
  dd 0x00
  dd - (0x1BADB002 + 0x00)

global start

extern kernel_entry

extern isr_common_handler
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

[global cpuid_is_supported]
cpuid_is_supported:
    pushfd                               ;Save EFLAGS
    pushfd                               ;Store EFLAGS
    xor dword [esp],0x00200000           ;Invert the ID bit in stored EFLAGS
    popfd                                ;Load stored EFLAGS (with ID bit inverted)
    pushfd                               ;Store EFLAGS again (ID bit may or may not be inverted)
    pop eax                              ;eax = modified EFLAGS (ID bit may or may not be inverted)
    xor eax,[esp]                        ;eax = whichever bits were changed
    popfd                                ;Restore original EFLAGS
    and eax,0x00200000                   ;eax = zero if ID bit can't be changed, else non-zero
    ret



[global cpuid_is_longmode_supported]
cpuid_is_longmode_supported:
	mov eax, 0x80000000    ; implicit argument for cpuid
	cpuid                  ; get highest supported argument
	cmp eax, 0x80000001    ; it needs to be at least 0x80000001
	jb .no_long_mode       ; if it's less, the CPU is too old for long mode

	; use extended info to test if long mode is available
	mov eax, 0x80000001    ; argument for extended processor info
	cpuid                  ; returns various feature bits in ecx and edx
	test edx, 1 << 29      ; test if the LM-bit is set in the D-register
	jz .no_long_mode       ; If it's not set, there is no long mode
	mov eax, 0x1
	ret
.no_long_mode:
	mov eax, 0
	ret	



[GLOBAL asm_load_page_directory_and_enable_paging]
asm_load_page_directory_and_enable_paging:
  push ebp
	mov ebp, esp

	mov eax, [esp + 8]
	mov cr3, eax

	mov eax, cr0
	or eax, 0x80000000

	mov esp, ebp
	pop ebp
	ret

asm_irq_common_handler:
  pushad

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

	popad
	add esp, 0x04
	sti
  iretd

asm_isr_common_handler:
  pusha

  mov ax, ds
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

  call isr_common_handler 
  
	pop ebx
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx

	popa
	add esp, 0x08
	sti
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
DEFINE_ISR_NOERROR 32

DEFINE_IRQ_HANDLER 0
DEFINE_IRQ_HANDLER 1

[GLOBAL enter_protected_mode]
enter_protected_mode:
  mov eax, cr0
	or al, 1
	mov cr0, eax
	mov ax, 0x10
	mov ds, ax 
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	jmp 8:flush_cpu
flush_cpu:
	ret

start:
  cli
	push ebx
  call kernel_entry

%if 0
mov esp, stack_space
section .bss
resb 8192
stack_space:
%endif
