bits 64

exit_message db 'exiting program'
message_length equ $-exit_message

global _start
global syscall_print_string 

extern main

syscall_print_string:
  mov rax, 0 
  int 0x80
  ret

_start:
  call main
  mov rax, 0x01
  int 0x80