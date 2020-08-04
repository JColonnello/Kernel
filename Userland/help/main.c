#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    FILE *file = fopen("userland/list.txt", "r");
    if(file == NULL)
    {
        fprintf(stderr, "No list.txt\n");
        return 1;
    }
    char buf[64];
    while(fgets(buf, sizeof(buf), file) != NULL)
    {
        buf[strlen(buf)-1] = 0;
        printf("%-20s", buf);
    }
    printf("\n");
    fclose(file);
    return 0;
}