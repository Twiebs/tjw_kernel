bits 64

%include "interrupt.asm"

global asm_longmode_entry
extern kernel_longmode_entry

asm_longmode_entry:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ax, 0x00
	mov ss, ax

	;Grub Multiboot info
	pop rdi ;Magic
	pop rsi ;Header Address

	call kernel_longmode_entry	
	hlt
