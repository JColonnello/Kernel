#include <platform.h>
#include <stdio.h>

void main()
{
	size_t phys, virt;
	memuse(&phys, &virt);
	printf("Memory use:\n");
	printf("\tPhysical memory: %lu bytes\n", phys);
	printf("\tKernel virtual memory: %lu bytes\n", virt);
}