#include <windows.h>
#include <list>
#include <atomic>

#include "dm_cmd.h"

dm_cmd_list::dm_cmd_list(dm_log* log)
{
    this->log = log;
    sem = false;
}

dm_cmd_list::~dm_cmd_list()
{
    dm_cmd* cmd;
    do
    {
        cmd = get();
        if(cmd != nullptr)
        {
            log->debug("cmd_list: drop cmd [%d]", cmd->type);
            next();
        }
    } while (cmd != nullptr);
}

void dm_cmd_list::add(dm_cmd* cmd)
{
    log->debug("cmd_list: add [%d]", cmd->type);

    bool sem_free_val = false;
    while(!sem.compare_exchange_strong(sem_free_val, true));
    
    cmd_list.push_back(cmd);

    sem.store(false);
}

dm_cmd* dm_cmd_list::get()
{
    bool sem_free_val = false;
    while(!sem.compare_exchange_strong(sem_free_val, true));

    dm_cmd* cmd = nullptr;
    if(!cmd_list.empty())
    {
        cmd = cmd_list.front();
        log->debug("cmd_list: get [%d]", cmd->type);
    }

    sem.store(false);

    return cmd;
}

void dm_cmd_list::next()
{
    bool sem_free_val = false;
    while(!sem.compare_exchange_strong(sem_free_val, true));

    dm_cmd* cmd;
    if(!cmd_list.empty())
    {
        cmd = cmd_list.front();
        log->debug("cmd_list: cmd removed [%d]", cmd->type);
        delete cmd;
        cmd_list.pop_front();
    }

    sem.store(false);
}

dm_cmd_exit_cmd_loop::dm_cmd_exit_cmd_loop()
{
    type = dm_cmd_type::exit_cmd_loop;
    attached = false;
}

dm_cmd_exit_cmd_loop::~dm_cmd_exit_cmd_loop()
{
}

dm_cmd_proc_show::dm_cmd_proc_show()
{
    type = dm_cmd_type::proc_show;
    attached = false;
}

dm_cmd_proc_show::~dm_cmd_proc_show()
{
}

dm_cmd_proc_run::dm_cmd_proc_run(char const* const path)
{
    UINT32 len = strlen(path);
    this->path = new char[len+1];
    strncpy(this->path, path, len);
    this->path[len] = 0;
    type = dm_cmd_type::proc_run;
    attached = false;
}

dm_cmd_proc_run::~dm_cmd_proc_run()
{
    delete[] path;
}

dm_cmd_proc_attach::dm_cmd_proc_attach(DWORD pid)
{
    this->pid = pid;
    type = dm_cmd_type::proc_attach;
    attached = false;
}

dm_cmd_proc_attach::~dm_cmd_proc_attach()
{
}

dm_cmd_proc_start::dm_cmd_proc_start()
{
    type = dm_cmd_type::proc_start;
    attached = true;
}

dm_cmd_proc_start::~dm_cmd_proc_start()
{
}

dm_cmd_proc_stop::dm_cmd_proc_stop()
{
    type = dm_cmd_type::proc_stop;
    attached = true;
}

dm_cmd_proc_stop::~dm_cmd_proc_stop()
{
}

dm_cmd_reg_read_u32::dm_cmd_reg_read_u32(UINT64 addr)
{
    this->addr = addr;
    type = dm_cmd_type::reg_read_u32;
    attached = true;
}
dm_cmd_reg_read_u32::~dm_cmd_reg_read_u32()
{
}

dm_cmd_reg_write_u32::dm_cmd_reg_write_u32(UINT32 val, UINT64 addr)
{
    this->val = val;
    this->addr = addr;
    type = dm_cmd_type::reg_write_u32;
    attached = true;
}
dm_cmd_reg_write_u32::~dm_cmd_reg_write_u32()
{
}

dm_cmd_scan_find_u32::dm_cmd_scan_find_u32(UINT32 val)
{
    this->val = val;
    type = dm_cmd_type::scan_find_u32;
    attached = true;
}
dm_cmd_scan_find_u32::~dm_cmd_scan_find_u32()
{
}
dm_cmd_scan_replace_u32::dm_cmd_scan_replace_u32(UINT32 val)
{
    this->val = val;
    type = dm_cmd_type::scan_replace_u32;
    attached = true;
}
dm_cmd_scan_replace_u32::~dm_cmd_scan_replace_u32()
{
}

dm_cmd_scan_reset_u32::dm_cmd_scan_reset_u32()
{
    type = dm_cmd_type::scan_reset_u32;
    attached = false;
}

dm_cmd_scan_reset_u32::~dm_cmd_scan_reset_u32()
{
}

