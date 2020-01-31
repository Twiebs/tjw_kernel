#!/bin/bash
source $(dirname $0)/variables.sh

mkdir -p $BINARY_OUTPUT_DIRECTORY

COMPILER_FLAGS="-g -O0"

nasm -fbin $KERNEL_SOURCE_DIRECTORY/secondary_cpu_init.asm -o $BINARY_OUTPUT_DIRECTORY/trampoline.bin 
$BINARY_OUTPUT_DIRECTORY/bin_to_txt $BINARY_OUTPUT_DIRECTORY/trampoline.bin $KERNEL_SOURCE_DIRECTORY/trampoline.txt
gcc $COMPILER_FLAGS -m64 -mno-red-zone -nostdlib -ffreestanding -fno-stack-protector -I $KERNEL_SOURCE_DIRECTORY -c $KERNEL_SOURCE_DIRECTORY/../build.c -o $BINARY_OUTPUT_DIRECTORY/build.c.o
nasm -felf64 $KERNEL_SOURCE_DIRECTORY/primary_cpu_init.asm -i$KERNEL_SOURCE_DIRECTORY -o $BINARY_OUTPUT_DIRECTORY/primary_cpu_init.asm.o
nasm -felf64 $KERNEL_SOURCE_DIRECTORY/x86_64_entry.asm -i$KERNEL_SOURCE_DIRECTORY -o $BINARY_OUTPUT_DIRECTORY/x86_64_entry.asm.o
ld -melf_x86_64 -n -T $KERNEL_SOURCE_DIRECTORY/../kernel_link.ld -o $BINARY_OUTPUT_DIRECTORY/kernel $BINARY_OUTPUT_DIRECTORY/build.c.o $BINARY_OUTPUT_DIRECTORY/x86_64_entry.asm.o $BINARY_OUTPUT_DIRECTORY/primary_cpu_init.asm.o
objcopy --only-keep-debug $BINARY_OUTPUT_DIRECTORY/kernel $BINARY_OUTPUT_DIRECTORY/debug_symbols
objcopy --strip-debug $BINARY_OUTPUT_DIRECTORY/kernel $BINARY_OUTPUT_DIRECTORY/kernel