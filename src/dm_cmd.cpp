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
            log->debug("cmd_list: cmd dropped [%d]", cmd->type);
            next();
        }
    } while (cmd != nullptr);
}

void dm_cmd_list::add(dm_cmd* cmd)
{
    cmd_list.push_back(cmd);
    log->debug("cmd_list: cmd added [%d]", cmd->type);
}

dm_cmd* dm_cmd_list::get()
{
    dm_cmd* cmd = nullptr;
    if(!cmd_list.empty())
    {
        cmd = cmd_list.front();
        log->debug("cmd_list: cmd get [%d]", cmd->type);
    }
    return cmd;
}

void dm_cmd_list::next()
{
    dm_cmd* cmd;
    if(!cmd_list.empty())
    {
        cmd = cmd_list.front();
        log->debug("cmd_list: cmd removed [%d]", cmd->type);
        delete cmd;
        cmd_list.pop_front();
    }
}

dm_cmd_exit_cmd_loop::dm_cmd_exit_cmd_loop()
{
    type = dm_cmd_type::exit_cmd_loop;
    attached = false;
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
    attached = false;
}

dm_cmd_start_process::~dm_cmd_start_process()
{
    delete[] path;
}

dm_cmd_fu32::dm_cmd_fu32(UINT32 val)
{
    this->val = val;
    type = dm_cmd_type::fu32;
    attached = true;
}
dm_cmd_fu32::~dm_cmd_fu32()
{
}
dm_cmd_fu32_replace::dm_cmd_fu32_replace(UINT32 val)
{
    this->val = val;
    type = dm_cmd_type::fu32_replace;
    attached = true;
}
dm_cmd_fu32_replace::~dm_cmd_fu32_replace()
{
}

dm_cmd_fu32_reset::dm_cmd_fu32_reset()
{
    type = dm_cmd_type::fu32_reset;
    attached = false;
}

dm_cmd_fu32_reset::~dm_cmd_fu32_reset()
{
}

dm_cmd_ru32::dm_cmd_ru32(UINT64 addr)
{
    this->addr = addr;
    type = dm_cmd_type::ru32;
    attached = true;
}
dm_cmd_ru32::~dm_cmd_ru32()
{
}

dm_cmd_wu32::dm_cmd_wu32(UINT32 val, UINT64 addr)
{
    this->val = val;
    this->addr = addr;
    type = dm_cmd_type::wu32;
    attached = true;
}
dm_cmd_wu32::~dm_cmd_wu32()
{
}