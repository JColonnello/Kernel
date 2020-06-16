#include <stdint.h>
#include <stdio.h>

void printreg(uint64_t *regs)
{
    char *names[16] = {"sp","bp","si","di","15","14","13","12","11","10","9","8","dx","cx","bx","ax"};
    for(int i = 0; i < 16; i++)
    {
        printf("%3s=%016lx", names[i], regs[i]);
    }
    printf("\n");
}