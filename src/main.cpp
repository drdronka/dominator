#include "dm_cmd.h"
#include "dm_log.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
    dm_log* log = new dm_log();
    dm_core* core = new dm_core(log);
    dm_cmd* cmd;

    if(argc > 1)
    {
        cmd = new dm_cmd(log, core, argv[1]);
    }
    else
    {
        cmd = new dm_cmd(log, core);
    }

    cmd->console();

    return 0;
}