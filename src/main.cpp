#include "dm_cmd.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
    dm_cmd* cmd;

    if(argc > 1)
    {
        cmd = new dm_cmd(argv[1]);
    }
    else
    {
        cmd = new dm_cmd();
    }

    cmd->console();

    return 0;
}