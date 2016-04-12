[BITS 32]
enter_real_mode:
  cli
	pop edx
	jmp 0x18:PM16


[BITS 16]
PM16:
  mov ax, 0x20
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov gs, ax
	mov fs, ax
	mov sp, 0x7c00+0x200  ;That will probably overwrite the kernels memory!

	;Clear out the protected bit
	mov eax, cr0
	and eax, 0xFFFFFFFE
	mov cr0, eax

	jmp 0x50:real_mode_entry

real_mode_entry:
  mov ax, 0x30
	mov ds, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov es, ax
	mov sp, 0x7c00+0x200

	cli
	lidt[.idtR]
	sti

