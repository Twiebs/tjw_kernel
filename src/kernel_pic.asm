
;NOTE(Torin) Considering using %defines here but i think
; The actualy port addressess inline might be more clear

%define PIC0_CTRL 0x20
%define PIC1_CTRL 0xA0

%define PIC0_DATA 0x21
%define PIC1_DATA 0xA1

pic_icw_init:
	; ICW 1
	; Initalization Bit and Expects ICW4
	mov al, 0x11
	out 0x20, al
	out 0xA0, al

	; The PIC is sent ICW2
	; It is told to use 32-47 IRQ 
	mov al, 0x20
	out 0x21, al
	mov al, 0x28
	out 0xA1, al

	; PIC uses IRQ 2 for connection to slave
	mov al, 0x4
	out 0x21, al
	;Slave gets the actual number of the IRQ used
	mov al, 0x2
	out 0xA1, al

	;ICW4 sets the PIC to 80x86 mode with bit 0
	mov al, 1
	out 0x21, al
	out 0xA1, al

	;Clear out the data registers
	mov al, 0
	out 0x21, 0
	out 0xA1, 0

pic_send_eoi:
	;Send EOI to primary PIC
	;This uses the OperationCommandWord2 Table
	;Bit 5 (0x20) is sent which indicates EOI
	mov al, 0x20
	out 0x20, al
