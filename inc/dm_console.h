#pragma once

#include "dm_log.h"
#include "dm_core.h"

enum class dm_console_err 
{
    ok = 0,
    get_arg = 1
};

class dm_console
{
    public:
        dm_console(dm_log* log_i, dm_core* core_i);
        dm_console(dm_log* log_i, dm_core* core_i, const char* path);
        ~dm_console();

        void run();

    protected:
        dm_console_err get_arg(char* cmd, UINT32 arg_n, char* arg);    
        
        dm_core* core;
        dm_log* log;
};