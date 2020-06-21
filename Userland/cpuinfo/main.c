#include <platform.h>
#include <stdio.h>

int main()
{
    char vendor[16], model[64];
    getcpuinfo(vendor, model);
    printf("CPU Vendor: %s\n", vendor);
    printf("Model: %s\n", model);

    return 0;
}