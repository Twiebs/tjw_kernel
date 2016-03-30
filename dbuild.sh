rm bin/kernel
nasm -f elf32 kernel_entry.asm -o kernel_entry.asm.o
g++ -std=c++11 -g -O0 -m32 -ffreestanding -Wall -Wextra -c kernel_entry.c -o kernel_entry.c.o
ld -m elf_i386 -T kernel_link.ld -o bin/kernel kernel_entry.asm.o kernel_entry.c.o

objcopy --only-keep-debug bin/kernel bin/debug_symbols
objcopy --strip-debug bin/kernel bin/kernel

rm *.o
