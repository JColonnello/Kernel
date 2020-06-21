#include <stdint.h>
#include <stdio.h>
#include <platform.h>

int main()
{
    char *names[16] = {"AX","BX","CX","DX","8","9","10","11","12","13","14","15","DI","SI","BP","SP"};

    RegisterStatus registers;
    dumpregs(&registers);
    uint64_t *regs = (void*)&registers;

    printf(" IP = %016lx\n\n", registers.rip);
    for(int i = 0; i < 16; i++)
    {
        printf("%3s=%016lx", names[i], regs[i]);
    }
    printf("\n");

    return 0;
}