
ENTRY(start)

SECTIONS 
{
	. = 1M;

	.boot :
	{
		*(.multiboot2_header)
	}


	.text ALIGN(4096): 
	{ 
		*(.text) 
	}

	.data :
	{
		*(.gdt)
	}

	.bss ALIGN(4096): 
	{
		*(.bss)
  }

	kernel_end = .;

}
