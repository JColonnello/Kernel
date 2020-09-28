#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/process.h>

int main(int argc, char *argv[])
{
    printf("Shell v1\n");
    for(;;)
    {
        printf("> ");
        char cmd[128];
        int s = scanf("%127[^\n]", cmd);
        while(getchar() != '\n');
        if(s < 1)
            continue;

        char *tokens[64];
        int i = 0;
        char *tok = strtok(cmd, " ");
        for(i = 0; i < sizeof(tokens)/sizeof(*tokens)-2 && tok != NULL; i++)
        {
            tokens[i] = tok;
            tok = strtok(NULL, " ");
        }
        tokens[i] = NULL;
        if(i == 0)
            continue;

        char path[64];
        sprintf(path, "userland/%s.bin", tokens[0]);
        bool back = false;
        char **params = &tokens[1];
        if(strcmp(tokens[i-1], "&") == 0)
        {
            tokens[i-1] = NULL;
            back = true;
        }
        int pid = execve(path, params, NULL);

        if(pid > 0)
        {
            if(!back)
                while(ispidrun(pid))
                    yield(pid);
        }
        else
            fprintf(stderr, "Command not found\n");
    }
    return 0;
}