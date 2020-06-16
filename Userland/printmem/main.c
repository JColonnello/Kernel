#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    uintptr_t dir;
    if(argc == 0)
        return 1;
    sscanf(argv[0], "%lx", &dir);
    printf("Memory at 0x%lx = 0x%016lx\n", dir, *(uintptr_t*)dir);
}