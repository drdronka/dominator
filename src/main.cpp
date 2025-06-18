#include "dm_console.h"
#include "dm_log.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
    dm_log* log = new dm_log();
    dm_core* core = new dm_core(log);
    dm_console* console;

    if(argc > 1)
    {
        console = new dm_console(log, core, argv[1]);
    }
    else
    {
        console = new dm_console(log, core);
    }

    console->run();

    delete console;
    delete core;
    delete log;

    return 0;
}