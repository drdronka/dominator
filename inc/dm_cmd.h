#pragma once

#include "dm_log.h"
#include "dm_core.h"

enum class dm_cmd_err 
{
    ok = 0,
    get_arg = 1
};

class dm_cmd
{
    public:

        dm_cmd(dm_log* log_i, dm_core* core_);
        dm_cmd(dm_log* log_i, dm_core* core_i, const char* path);
        ~dm_cmd();

        void console();

    protected:

        dm_core* core;
        dm_log* log;

        dm_cmd_err get_arg(char* cmd, UINT32 arg_n, char* arg);    
};