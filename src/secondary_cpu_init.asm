[BITS 16]

%define P4_TABLE 0x10F000

%define CR0_PAGING_ENABLED_BIT (1 << 31)

%define CR4_PAE_BIT (1 << 5) ;Physical Address Extention
%define CR4_PGE_BIT (1 << 7) ;Page Gobal Enable

%define MSR_LONG_MODE_BIT (1 << 8)
%define EFLAGS_ID_BIT (1 << 21)
%define EFER_MSR (0xC0000080)

%define PAGING_PRESENT_BIT 			(1 << 0)
%define PAGING_WRITEABLE_BIT 	 	(1 << 1)
%define PAGING_HUGE_BIT 				(1 << 7) 
ap_init_procedure:
  cli
  mov ax, cs
  mov ds, ax

  lgdt[GDT64.Pointer]

  mov al, 0xFF
  out 0xA1, al
  out 0x21, al
  lidt[IDT]

  mov eax, P4_TABLE 
	mov cr3, eax

	mov eax, cr4                 
	or eax, CR4_PAE_BIT
	mov cr4, eax

	mov ecx, EFER_MSR
	rdmsr
	or eax, MSR_LONG_MODE_BIT
	wrmsr   

	mov eax, cr0                 
	or eax, CR0_PAGING_ENABLED_BIT | 0x01
	mov cr0, eax
  jmp 0x08:long_mode + 0x1000 

bits 64
long_mode:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
  mov ax, 0x00
	mov ss, ax
  xchg bx, bx

  xchg bx, bx
  mov rax, [0x2000]
  mov rsp, 0x2000 
  call rax
  hlt

ALIGN 4
IDT:
  .Length dw 0
  .Base dd 0

GDT64:                         
.null:
	dq 0
.code:
	dq 0x0020980000000000 
.data:
  dq 0x0000900000000000 
.Pointer:                      ; The GDT-pointer.
	dw $ - GDT64 - 1             ; Limit.
	dd GDT64 + 0x1000            ; Base.