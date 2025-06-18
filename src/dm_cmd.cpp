#include <windows.h>

#include "dm_cmd.h"

dm_cmd_list::dm_cmd_list(dm_log* log)
{
    this->log = log;
}

dm_cmd_list::~dm_cmd_list()
{
    dm_cmd* cmd;
    do
    {
        cmd = get();
        if(cmd != nullptr)
        {
            log->debug("cmd dropped [%d]", cmd->type);
            next();
        }
    } while (cmd != nullptr);
}

void dm_cmd_list::add(dm_cmd* cmd)
{
    cmd_list.push_back(cmd);
    log->debug("cmd added [%d]", cmd->type);
}

dm_cmd* dm_cmd_list::get()
{
    dm_cmd* cmd = nullptr;
    if(!cmd_list.empty())
    {
        cmd = cmd_list.front();
        log->debug("cmd get [%d]", cmd->type);
    }
    return cmd;
}

void dm_cmd_list::next()
{
    dm_cmd* cmd;
    if(!cmd_list.empty())
    {
        cmd = cmd_list.front();
        log->debug("cmd removed [%d]", cmd->type);
        delete cmd;
        cmd_list.pop_front();
    }
}

dm_cmd_exit_cmd_loop::dm_cmd_exit_cmd_loop()
{
    type = dm_cmd_type::exit_cmd_loop;
}

dm_cmd_exit_cmd_loop::~dm_cmd_exit_cmd_loop()
{
}

dm_cmd_start_process::dm_cmd_start_process(char const* const path)
{
    UINT32 len = strlen(path);
    this->path = new char[len+1];
    strncpy(this->path, path, len);
    this->path[len] = 0;

    type = dm_cmd_type::start_process;
}

dm_cmd_start_process::~dm_cmd_start_process()
{
    delete[] path;
}