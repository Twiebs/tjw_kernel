
ENTRY(start)

SECTIONS 
{

	. = 1M;
	.multiboot_header :
	{
		*(.multiboot2_header)
	}

	.kernel_code ALIGN(4096): 
	{ 
		*(.text) 
	}

	.kernel_data_read_write ALIGN(4096):
	{
		*(.data)
	}

	.kernel_data_read_only ALIGN(4096): 
	{
	    *(.rodata)
	}

	.kernel_bss_data ALIGN(4096):
	{
		*(.bss)
	}
			
	_KERNEL_END = .;
}
