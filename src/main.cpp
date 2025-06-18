#include "dm_console.h"
#include "dm_log.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
    dm_log* log = new dm_log();
    dm_core* core = new dm_core(log);

    if(argc > 1)
    {
        core->add_cmd((dm_cmd*)new dm_cmd_start_process(argv[1]));
    }
    
    dm_console* console = new dm_console(log, core);
    console->run();

    //delete console;
    delete core;
    delete log;

    return 0;
}