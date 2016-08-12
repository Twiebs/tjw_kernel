%define MULTIBOOT2_MAGIC_NUMBER 0xE85250D6
%define MULTIBOOT2_ARCHITECTURE 0

%define MULTIBOOT2_TAG_TYPE_FRAMEBUFFER 5
%define MULTIBOOT2_TAGTYPE_END 0

section .multiboot2_header
header_start:
  dd MULTIBOOT2_MAGIC_NUMBER
	dd MULTIBOOT2_ARCHITECTURE 
	dd header_end - header_start
	dd (1 << 32) - (MULTIBOOT2_MAGIC_NUMBER + MULTIBOOT2_ARCHITECTURE + (header_end - header_start))

%if 1
  ;Framebuffer Tag
  dw 5 ;Framebuffer tag type
  dw 0 ;flags
  dd 20 ;size
  dd 1280 ;width
  dd 720  ;height
  dd 24   ;depth
%endif

%if 1
	;End Tag
	dw 0
	dw 0
	dd 0
%endif


header_end: