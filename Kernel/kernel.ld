ENTRY(loader)
SECTIONS
{
	__startOfUniverse = 0xFFFFFFFFC0000000;
	.shared :
	{
		__test_sync_global = __startOfUniverse + 0x8000;
		__test_phylo_global = __startOfUniverse + 0x8010;
	}
	. = __startOfUniverse;
	__vga = . + 0xA0000 + 0x18000;
	.text_load __startOfUniverse + 0x100000 :
	{
		*(.text_load*)
	}
	__startOfKernel = .;
	.text :
	{
		__text = .;
		*(.text*)
		. = ALIGN(0x1000);
		__rodata = .;
		*(.rodata*)
	}
	.data ALIGN(0x1000) : AT(ADDR(.data))
	{
		__data = .;
		*(.data*)
		__endOfKernelBinary = .;
	}
	.bss ALIGN(0x1000) : AT(ADDR(.bss))
	{
		__bss = .;
		*(.bss*)
		*(EXCLUDE_FILE (*.o) COMMON)
	}
	. = ALIGN(0x1000);
	__endOfKernel = .;
	__endOfKernelStack = . + 0x1000 * (1 + 16);
}
