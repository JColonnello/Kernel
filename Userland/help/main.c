#include <stdio.h>
#include <stdlib.h>

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
        printf("%s", buf);
    }
    fclose(file);
}