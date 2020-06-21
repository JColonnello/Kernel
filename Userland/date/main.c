#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <platform.h>

int main()
{
    struct tm time;
    date(&time);
    char *str = asctime(&time);

    printf("%s\n", str);
    free(str);
    
    return 0;
}