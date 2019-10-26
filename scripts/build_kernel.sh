#!/bin/bash
SCRIPT_DIR=$(dirname $0)
OUTPUT_DIRECTORY=$SCRIPT_DIR/../intermediate/binaries
COMPILER_FLAGS=-g -O0

mkdir -p $OUTPUT_DIRECTORY
nasm -fbin src/secondary_cpu_init.asm -o $OUTPUT_DIRECTORY/trampoline.bin 
$OUTPUT_DIRECTORY/bin_to_txt $OUTPUT_DIRECTORY/trampoline.bin src/trampoline.txt
gcc $COMPILER_FLAGS -m64 -mno-red-zone -nostdlib -ffreestanding -fno-stack-protector -I ../tjw_kernel/src -c build.c -o $OUTPUT_DIRECTORY/build.c.o
nasm -felf64 src/primary_cpu_init.asm -isrc/ -o $OUTPUT_DIRECTORY/primary_cpu_init.asm.o
nasm -felf64 src/x86_64_entry.asm -isrc/ -o $OUTPUT_DIRECTORY/x86_64_entry.asm.o
ld -melf_x86_64 -n -T kernel_link.ld -o $OUTPUT_DIRECTORY/kernel $OUTPUT_DIRECTORY/build.c.o $OUTPUT_DIRECTORY/x86_64_entry.asm.o $OUTPUT_DIRECTORY/primary_cpu_init.asm.o
objcopy --only-keep-debug $OUTPUT_DIRECTORY/kernel $OUTPUT_DIRECTORY/debug_symbols
objcopy --strip-debug $OUTPUT_DIRECTORY/kernel $OUTPUT_DIRECTORY/kernel
