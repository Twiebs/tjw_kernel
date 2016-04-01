OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS {
	
	.text 0x100000: { 
		*(.text) 
	}

	.data : {
		*(.data) 
	}

	.bss  : { 
		*(.bss)
  }

	kernelend = .;
}
