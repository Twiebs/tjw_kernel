%define MULTIBOOT2_MAGIC_NUMBER 0xE85250D6
%define MULTIBOOT2_ARCHITECTURE 0

%define MULTIBOOT2_TAGTYPE_END 0

section .multiboot2_header
header_start:
  dd MULTIBOOT2_MAGIC_NUMBER
	dd MULTIBOOT2_ARCHITECTURE 
	dd header_end - header_start
	dd (1 << 32) - (MULTIBOOT2_MAGIC_NUMBER + MULTIBOOT2_ARCHITECTURE + (header_end - header_start))

	;End Tag
	dw MULTIBOOT2_TAGTYPE_END
	dw 0
	dd 8
header_end:
