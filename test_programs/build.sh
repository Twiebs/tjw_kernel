nasm -felf64 syslib.asm -o syslib.o
clang -c -ffreestanding test.c -o test.o 
ld test.o syslib.o -o result.o 
../bin/bin_to_txt result.o ../src/test_program.txt 
#rm *.o