#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/process.h>
#include <platform.h>
#include <syslib.h>

struct process
{
    char path[64];
    char **params;
    int count;
    bool background;
};

int parseTokens(char **toks)
{
    int i;
    int pid = -1;
    int in = dup(0);
    int out = dup(1);
    int nextIn = -1;
    struct process p1;
    for(i = 0; toks[i] != NULL;)
    {
        p1 = (struct process){0};
        snprintf(p1.path, sizeof(p1.path), "userland/%s.bin", toks[i++]);
        p1.params = &toks[i];
        while(toks[i] != NULL)
        {
            if(strcmp(toks[i], "&") == 0)
            {
                p1.background = true;
                i++;
                break;
            }
            if(strcmp(toks[i], "|") == 0)
                break;
            else
            {
                p1.count++;
                i++;
            }
        }
        if(toks[i] != NULL)
        {
            if(strcmp(toks[i], "|") == 0)
            {
                int fd[2];
                pipe(fd);

                close(1);
                dup(fd[1]);
                close(fd[1]);

                nextIn = fd[0];
                i++;
            }
            else if(!p1.background)
            {
                fprintf(stderr, "Syntax error\n");
                break;
            }
        }
        char *params[p1.count + 1];
        int j;
        for(j = 0; j < p1.count; j++)
            params[j] = p1.params[j];
        params[j] = NULL;
        pid = execve(p1.path, params, NULL);
        if(p1.background)
            setjobstatus(pid, JOB_BACKGROUND);
        if(nextIn > 0)
        {
            close(0);
            dup(nextIn);
            close(nextIn);
            nextIn = -1;
        }
        close(1);
        dup(out);
        if(pid <= 0)
        {
            fprintf(stderr, "Command not found\n");
            break;
        }
    }
    close(0);
    close(1);
    dup(in);
    dup(out);
    close(in);
    close(out);
    return !p1.background ? pid : -1;
}

int main(int argc, char *argv[])
{
    printf("Shell v1\n");
    for(;;)
    {
        fprintf(stderr, "> ");
        char cmd[128];
        if(fgets(cmd, sizeof(cmd), stdin) == NULL)
        {
            fprintf(stderr, "EOF\n");
            break;
        }
        cmd[strlen(cmd)-1] = 0;

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

        int pid = parseTokens(tokens);

        if(pid > 0)
        {
            while(ispidrun(pid))
                yield(pid);
        }
    }
    return 0;
}