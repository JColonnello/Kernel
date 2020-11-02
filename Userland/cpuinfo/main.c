#include <platform.h>
#include <stdio.h>

int main()
{
    char vendor[16], model[64];
    getcpuinfo(vendor, model);
    printf("CPU Vendor: %s\n", vendor);
    printf("Model: %s\n", model);

    uint32_t features[4];
    cpuid(features, 0x01);
    printf("0x01:\tEAX=%08x\tEBX=%08x\tECX=%08x\tEDX=%08x\t\n", features[0], features[1], features[2], features[3]);
    cpuid(features, 0x80000001);
    printf("0x80000001:\tEAX=%08x\tEBX=%08x\tECX=%08x\tEDX=%08x\t\n", features[0], features[1], features[2], features[3]);
    return 0;
}