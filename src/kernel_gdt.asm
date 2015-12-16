[GLOBAL gdt_flush]    ; Allows the C code to call gdt_flush().

gdt_flush:
   mov eax, [esp+4]  ; Get the pointer to the GDT, passed as a parameter.
   lgdt [eax]        ; Load the new GDT pointer

   mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
   mov ds, ax        ; Load all data segment selectors
   mov es, ax
   mov fs, ax
   mov gs, ax
   mov ss, ax
   jmp 0x08:.flush   ; 0x08 is the offset to our code segment: Far jump!
.flush:
   ret 


[GLOBAL idt_flush]

idt_flush:
	mov eax, [esp+4] ;Pointer to the IDT 
	lidt [eax]
	ret

; Note(Torin) Takes 1 parameter %s Accesses it 
%macro ISR_NO_ERROR_CODE 1
	[GLOBAL isr%1]
	isr%1:
		cli ; Clear Interrupts
		push byte 0
		push byte %1
		jmp isr_common_stub
%endmacro

%macro ISR_ERROR_CODE 1
	[GLOBAL isr%1]
	isr%1:
		cli
		push byte %1
		jmp isr_common_stub
%endmacro

ISR_NO_ERROR_CODE 0
ISR_NO_ERROR_CODE 1
ISR_NO_ERROR_CODE 2
ISR_NO_ERROR_CODE 3
ISR_NO_ERROR_CODE 4
ISR_NO_ERROR_CODE 5
ISR_NO_ERROR_CODE 6
ISR_NO_ERROR_CODE 7
ISR_ERROR_CODE 8
ISR_NO_ERROR_CODE 9
ISR_ERROR_CODE 10
ISR_ERROR_CODE 11
ISR_ERROR_CODE 12
ISR_ERROR_CODE 13
ISR_ERROR_CODE 14
ISR_NO_ERROR_CODE 15
ISR_NO_ERROR_CODE 16
ISR_NO_ERROR_CODE 17
ISR_NO_ERROR_CODE 18
ISR_NO_ERROR_CODE 19
ISR_NO_ERROR_CODE 20
ISR_NO_ERROR_CODE 21
ISR_NO_ERROR_CODE 22
ISR_NO_ERROR_CODE 23
ISR_NO_ERROR_CODE 24
ISR_NO_ERROR_CODE 25
ISR_NO_ERROR_CODE 26
ISR_NO_ERROR_CODE 27
ISR_NO_ERROR_CODE 28
ISR_NO_ERROR_CODE 29
ISR_NO_ERROR_CODE 30
ISR_NO_ERROR_CODE 31

[EXTERN isr_handler]

isr_common_stub:
	pusha ;Pushes the general registers

	mov ax, ds ;Lower 16 bits of eax = ds
	push eax ; sae data segment descriptor

	mov ax, 0x10 ; load kernel data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	call isr_handler ;Call the C isr_handle in the kernel

	pop ebx; reloads the orginal data segment descriptor
	mov ds, bx 
	mov es, bx 
	mov fs, bx 
	mov gs, bx 

	popa ; pop the general registers
	add esp, 8 ; Cleans up the pushed error code and pushed ISR number by incrementing the stack pointer
	sti
	iret



