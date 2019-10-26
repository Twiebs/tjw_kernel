bits 16
org 0x7c00 ;this is where the bios puts the bootloader in memory. It's (2^15 - 1024)

boot:
    mov si, HELLO_WORLD_STRING
    mov ah, 0x0e
.loop:
    lodsb
    or al, al
    jz halt 
    int 0x10
    jmp .loop
halt:
    cli
    hlt

HELLO_WORLD_STRING: db "Hello world!", 0