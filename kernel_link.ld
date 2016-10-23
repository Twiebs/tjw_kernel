
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
		*(.data)
	}

	.bss ALIGN(4096): 
	{
		*(.bss)
	}

	_KERNEL_END = .;
}
