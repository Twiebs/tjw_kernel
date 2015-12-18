file(REMOVE_RECURSE
  "CMakeFiles/kernel.dir/kernel_main.c.o"
  "CMakeFiles/kernel.dir/kernel_terminal.c.o"
  "CMakeFiles/kernel.dir/kernel_descriptor_table.c.o"
  "CMakeFiles/kernel.dir/kernel_boot.asm.o"
  "CMakeFiles/kernel.dir/kernel_gdt.asm.o"
  "kernel.pdb"
  "kernel.bin"
)

# Per-language clean rules from dependency scanning.
foreach(lang ASM_NASM C)
  include(CMakeFiles/kernel.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
