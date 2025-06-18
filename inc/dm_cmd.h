#pragma once

#include <list>

#include "dm_log.h"

enum class dm_cmd_type
{
    exit_cmd_loop = 0,
    start_process = 1,
    attach_to_process = 1,
    mem_scan = 2,
};

class dm_cmd
{
    public:    
        dm_cmd_type type;
};

class dm_cmd_list
{
    public:
        dm_cmd_list(dm_log* log);
        ~dm_cmd_list();

        void add(dm_cmd* cmd);
        dm_cmd* get();
        void next();

        std::list<dm_cmd*> cmd_list;

    protected:
        dm_log* log;
};

class dm_cmd_exit_cmd_loop : dm_cmd
{
    public:
        dm_cmd_exit_cmd_loop();
        ~dm_cmd_exit_cmd_loop();
};

class dm_cmd_start_process : dm_cmd
{
    public:
        dm_cmd_start_process(char const* const path);
        ~dm_cmd_start_process();

        char* path;
};