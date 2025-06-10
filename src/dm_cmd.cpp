#include <stdio.h>
#include <windows.h>

#include "dm_cmd.h"
#include "dm_core.h"

#define CMD_SIZE 256

const char target_app[] = "C:\\Users\\Dron\\test\\test.exe";

dm_cmd::dm_cmd()
{
}

void dm_cmd::console()
{
    UINT8 loop_exit = 0;
    dm_core core;

    printf("dominator cmd line\n");

    while(!loop_exit)
    {
        char cmd[CMD_SIZE];

        printf("> ");
        fflush(stdout);

        fgets(cmd, CMD_SIZE, stdin);

        if(!strncmp(cmd, "run", strlen("run")))
        {
            core.run(target_app);
        }
        else if(!strncmp(cmd, "exit", strlen("exit")))
        {
            loop_exit = 1;
        }
        else 
        {
            if(strlen(cmd) > 0) cmd[strlen(cmd)-1]=0;
            printf("unknown command: [%s]\n", cmd);
        }
    }
}
