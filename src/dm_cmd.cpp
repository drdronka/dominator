#include <stdio.h>
#include <windows.h>

#include "dm_cmd.h"
#include "dm_core.h"

#define CMD_SIZE 256

dm_cmd::dm_cmd()
{
    core = new dm_core();
}

dm_cmd::dm_cmd(const char* path)
{
    core = new dm_core();
    core->start_process(path);
}

dm_cmd::~dm_cmd()
{
    delete core;
}

void dm_cmd::console()
{
    UINT8 loop_exit = 0;

    printf("dominator cmd line\n");

    while(!loop_exit)
    {
        char cmd[CMD_SIZE];
        char buff[CMD_SIZE];

        printf("> ");
        fflush(stdout);

        fgets(cmd, CMD_SIZE, stdin);
        cmd[strlen(cmd)-1]=0;

        if(!strncmp(cmd, "run", strlen("run")))
        {
            arg_strip(cmd + strlen("run") + 1, buff);
            core->start_process(buff);
        }
        else if(!strncmp(cmd, "exit", strlen("exit")))
        {
            loop_exit = 1;
        }
        else 
        {
            printf("unknown command: [%s]\n", cmd);
        }
    }
}

void dm_cmd::arg_strip(char* in, char* out)
{    
    UINT32 out_n = 0;
    for(UINT32 in_n = 0; in_n < strlen(in); in_n++)
    {
        if(in[in_n] != ' ' && in[in_n] != '\"' && in[in_n] != '\'')
        {
            out[out_n] = in[in_n];
            out_n++;
        }
    }
}