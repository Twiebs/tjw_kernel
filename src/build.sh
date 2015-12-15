nasm -felf32 kernel_boot.asm -o boot.o
nasm -felf32 kernel_gdt.asm -o gdt.o
i386-linux-gcc -c kernel_main.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i386-linux-gcc -T linker.ld -o tjw_os.bin -ffreestanding -O2 -nostdlib boot.o gdt.o kernel.o -lgcc
