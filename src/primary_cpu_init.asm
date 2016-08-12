bits 32

global start

extern asm_longmode_entry
extern kernel_end

%include "multiboot2_header.asm"

global GDT64

section .gdt
GDT64:                         
.null:
	dq 0
.code:
	dq 0x0020980000000000 
.data:
  dq 0x0000900000000000 
.Pointer:                    ; The GDT-pointer.
	dw $ - GDT64 - 1             ; Limit.
	dd GDT64                     ; Base.

section .text

start:
  xchg bx, bx
	cli
	mov esp, stack_top
	push 0x0
	push ebx
	push 0x0
	push eax
	mov edi, ebx

	call is_multiboot2_bootloader
	call is_cpuid_supported
	call is_longmode_supported
	
	;call fill_temporary_paging_tables
	call setup_paging_tables
	call enable_paging

%if 0
  ;Enable SSE2
  mov eax, cr0
  and ax, 0xFFFB
  or ax, 0x2
  mov cr0, eax
  mov eax, cr4
  or ax, 3 << 9
  mov cr4, eax
%endif

	lgdt [GDT64.Pointer]
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	jmp 0x8:asm_longmode_entry

setup_paging_tables:
	%define PAGING_PRESENT_BIT 			(1 << 0)
	%define PAGING_WRITEABLE_BIT 	 	(1 << 1)
	%define PAGING_USER_ACCESSIBLE 	(1 << 2)
	%define PAGING_WRITE_THFROUGH_CACHING (1 << 3)
	%define PAGING_DISABLE_CACHE (1 << 4)
	%define PAGING_HUGE_BIT 				(1 << 7)

	mov eax, g_p3_table
	or eax, (PAGING_PRESENT_BIT | PAGING_WRITEABLE_BIT) 
	mov [g_p4_table], eax

	mov eax, g_p2_table
	or eax, (PAGING_PRESENT_BIT | PAGING_WRITEABLE_BIT)
	mov [g_p3_table], eax

	;Identity map the bottom 2MB for the kernels use
	mov eax, 0x00 
	or eax, (PAGING_PRESENT_BIT | PAGING_WRITEABLE_BIT | PAGING_HUGE_BIT)
	mov [g_p2_table], eax
	ret
 
enable_paging:
	%define CR0_PAGING_ENABLED_BIT (1 << 31)
	%define CR4_PHYSICAL_ADDRESS_EXTENTION_BIT (1 << 5)
	%define MSR_LONG_MODE_BIT (1 << 8)
	%define EFLAGS_ID_BIT (1 << 21)
	%define EFER_MSR (0xC0000080)

	mov eax, g_p4_table
	mov cr3, eax

	mov eax, cr4                 
	or eax, CR4_PHYSICAL_ADDRESS_EXTENTION_BIT 
	mov cr4, eax 

	mov ecx, EFER_MSR
	rdmsr           
	or eax, MSR_LONG_MODE_BIT 
	wrmsr   

	mov eax, cr0                 
	or eax, CR0_PAGING_ENABLED_BIT 
	mov cr0, eax                 
	ret
	
; System Feature Checks
;============================================

%define ERROR_CODE_NO_MULTIBOOT "0"
%define ERROR_CODE_NO_CPUID "7"
%define ERROR_CODE_NO_LONGMODE "2"

is_multiboot2_bootloader:
  cmp eax, 0x36D76289
	jne .no_multiboot
	ret
.no_multiboot:
  mov al, ERROR_CODE_NO_MULTIBOOT 
	jmp error_handler

;CPUID Support
is_cpuid_supported: 
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21) in
    ; the FLAGS register. If we can flip it, CPUID is available.
    ; Copy FLAGS in to EAX via stack
    pushfd
    pop eax
 
    ; Copy to ECX as well for comparing later on
    mov ecx, eax
 
    ; Flip the ID bit
    xor eax, 1 << 21
 
    ; Copy EAX to FLAGS via the stack
    push eax
    popfd
 
    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushfd
    pop eax
 
    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the ID bit
    ; back if it was ever flipped).
    push ecx
    popfd
 
    ; Compare EAX and ECX. If they are equal then that means the bit wasn't
    ; flipped, and CPUID isn't supported.
    xor eax, ecx
    jz .no_cpuid 
    ret
.no_cpuid:
  mov al, ERROR_CODE_NO_CPUID
	jmp error_handler

is_longmode_supported:
  mov eax, 0x80000000
	cpuid
	cmp eax, 0x80000001
	jb .no_longmode
	mov eax, 0x80000001
	cpuid
	test edx, 1 << 29
	jz .no_longmode
	ret
.no_longmode:
  mov al, ERROR_CODE_NO_LONGMODE
	jmp error_handler

error_handler:
	mov dword [0xb8000], 0x4f524f45
	mov dword [0xb8004], 0x4f3a4f52
	mov dword [0xb8008], 0x4f204f20
	mov byte  [0xb800a], al
	hlt

global g_p4_table
global g_p3_table
global g_p2_table

section .bss
align 4096
g_p4_table:
  resb 4096
align 4096
g_p3_table:
  resb 4096
align 4096
g_p2_table:
  resb 4096
g_cpu_counter:
  resb 2
stack_bottom:
  resb 8192
stack_top:
