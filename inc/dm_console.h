#pragma once

#include "dm_log.h"
#include "dm_core.h"

class dm_console
{
    public:
        dm_console(dm_log* log_i, dm_core* core_i);
        ~dm_console();

        void run();

    protected:
        bool get_arg(char const* const cmd, UINT32 arg_n, char* arg);    
        bool is_arg(char const* const cmd, char const* const arg);
        
        dm_core* core;
        dm_log* log;
};