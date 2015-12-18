cmake_minimum_required(VERSION 2.8)
project(tjw-kernel)

enable_language(ASM_NASM)
set(CMAKE_C_COMPILER i386-linux-gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -ffreestanding -O2 -Wall -Wextra")
set(CMAKE_EXECUTABLE_SUFFIX ".bin")
set(CMAKE_ASK_NASM_FLAGS -felf32)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> ${CMAKE_ASK_NASM_FLAGS} <SOURCE> -o <OBJECT>")

set(KERNEL_SOURCES
	kernel_main.c
	kernel_terminal.c
	kernel_descriptor_table.c)

set(KERNEL_ASM_SOURCES
	kernel_boot.asm
	kernel_gdt.asm)

set(KERNEL_LINK_FLAGS
	-T linker.ld
	-ffreestanding
	-O2
	-nostdlib
	-lgcc)

set(KERNEL_LINKER_SCRIPT ../linker.ld)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${KERNEL_LINKER_SCRIPT} -ffreestanding -O2 -nostdlib -lgcc")
add_executable(kernel ${KERNEL_SOURCES} ${KERNEL_ASM_SOURCES})




