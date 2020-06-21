#include <stdio.h>
#include <platform.h>

int main()
{
    unsigned char currTemp, maxTemp;
    temp(&currTemp, &maxTemp);
    printf("Current temperature: %d°C\n", currTemp);
    printf("Max temperature: %d°C\n", maxTemp);

    return 0;
}