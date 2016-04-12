MULTIBOOT_FLAG_PAGE_ALIGN    equ 1 << 0    
MULTIBOOT_FLAG_MEMORY_INFO   equ 1 << 1    
MULTIBOOT_FLAG_GRAPHICS      equ 1 << 2

MBOOT_HEADER_FLAGS  equ (MBOOT_FLAG_PAGE_ALIGN | MBOOT_FLAG_GRAPHICS)
MBOOT_HEADER_MAGIC equ 0x1BADB002 ; Multiboot Magic value
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[GLOBAL mboot]                  ; Make 'mboot' accessible from C.
[EXTERN code]                   ; Start of the '.text' section.
[EXTERN bss]                    ; Start of the .bss section.
[EXTERN end]                    ; End of the last loadable section.

mboot:
  dd  MBOOT_HEADER_MAGIC        ; GRUB will search for this value on each
  dd  MBOOT_HEADER_FLAGS        ; How GRUB should load your file / settings
  dd  MBOOT_CHECKSUM            ; To ensure that the above values are correct
   
  dd  mboot                     ; Location of this descriptor
  dd  code                      ; Start of kernel '.text' (code) section.
  dd  bss                       ; End of kernel '.data' section.
  dd  end												; End of kernel.
  dd  start                     ; Kernel entry point (initial EIP).
	
	dd 0x0 ;Linear Framebuffer
	dd 0x0 ;0 Width 
	dd 0x0 ;0 Height
	dd 32 ;32 Depth
