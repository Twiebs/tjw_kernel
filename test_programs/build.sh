nasm -felf64 syslib.asm -o syslib.o
clang -c -ffreestanding test.c -o test.o
ld test.o syslib.o -o test_program
