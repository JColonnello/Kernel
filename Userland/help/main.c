#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void commandHelp(char *cmd)
{
    char buf[128];
    sprintf(buf, "help/%s.txt", cmd);
    FILE *file = fopen(buf, "r");
    if(file == NULL)
    {
        fprintf(stderr, "No information available\n");
        return;
    }
    int count;
    while((count = fread(buf, 1, sizeof(buf), file)) != 0)
        fwrite(buf, 1, count, stdout);
    fclose(file);
}

int main(int argc, char *args[])
{
    if(argc == 1)
    {
        commandHelp(args[0]);
        return 0;
    }

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