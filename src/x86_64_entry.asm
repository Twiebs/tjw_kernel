bits 64

%include "interrupt_handler.asm"

extern kernel_longmode_entry

global asm_longmode_entry
global asm_enter_usermode

%define GDT_RING0_DATA 0x08
%define GDT_RING3_DATA 0x10
%define GDT_RING0_CODE 0x18
%define GDT_RING3_CODE 0x20
%define GDT_TSS 0x28

asm_longmode_entry:
	mov ax, GDT_RING0_DATA
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ax, 0x00
	mov ss, ax

	;Grub Multiboot info
	pop rdi ;Multiboot Magic Number
	pop rsi ;Multiboot Header Address
	call kernel_longmode_entry

;TODO(Torin) Check if we need to align on a 16 byte boundary
asm_enter_usermode:
  cli

  %if 1
  mov ax, (GDT_RING3_DATA | 0b11) ;Data Segment ring 3 selector
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  %endif

  push (GDT_RING3_DATA | 0x3) ;Push SS
  push rsi ;Push Return RSP
  pushfq     ;Push Rflags
  push (GDT_RING3_CODE | 0x3) ;Push CS
  push rdi ;address to execute
  iretq