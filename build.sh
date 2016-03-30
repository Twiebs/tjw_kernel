rm bin/kernel
nasm -f elf32 kernel_entry.asm -o kernel_entry.asm.o
g++ -std=c++11 -m32 -ffreestanding -Wall -Wextra -c kernel_entry.c -o kernel_entry.c.o
ld -m elf_i386 -T kernel_link.ld -o bin/kernel kernel_entry.asm.o kernel_entry.c.o
rm *.o
