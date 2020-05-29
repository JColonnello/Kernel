OUTPUT_FORMAT("binary")
ENTRY(loader)
SECTIONS
{
	startOfUniverse = 0xFFFFFFFFC0000000;
	. = startOfUniverse;
	vga = . + 0xA0000;
	.text_load startOfUniverse + 0x100000 :
	{
		*(.text_load*)
	}
	startOfKernel = .;
	.text :
	{
		text = .;
		*(.text*)
		. = ALIGN(0x1000);
		rodata = .;
		*(.rodata*)
	}
	.data ALIGN(0x1000) : AT(ADDR(.data))
	{
		data = .;
		*(.data*)
		endOfKernelBinary = .;
	}
	.bss ALIGN(0x1000) : AT(ADDR(.bss))
	{
		bss = .;
		*(.bss*)
		*(EXCLUDE_FILE (*.o) COMMON)
	}
	. = ALIGN(0x1000);
	endOfKernel = .;
}
