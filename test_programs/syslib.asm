bits 64


global _start
;global syscall_print_string 

extern main

%if 0
syscall_print_string:
  mov rax, 0 
  int 0x80
  ret
%endif

_start:
  call main
  mov rax, 0x01
  int 0x80