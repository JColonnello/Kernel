#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    printf("Shell v1\n");
    for(;;)
    {
        printf("> ");
        char cmd[32];
        scanf("%31[^\n]", cmd);
        while(getchar() != '\n');

        char path[64];
        char *param = strchr(cmd,' ');
        if(param != NULL)
        {
            *param = 0;
            param++;
        }
        sprintf(path, "userland/%s.bin", cmd);

        char *params[] = {param, NULL};
        int pid = execve(path, params, NULL);
        if(pid > 0)
            wait(pid);
        else
            fprintf(stderr, "Command not found\n");
    }
    return 0;
}