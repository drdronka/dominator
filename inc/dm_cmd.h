#pragma once

#include <list>
#include <atomic>

#include "dm_log.h"

enum class dm_cmd_type
{
    exit_cmd_loop = 0,
    proc_show = 100,
    proc_run = 101,
    proc_attach = 102,
    proc_start = 103,
    proc_stop = 104,
    reg_read_u32 = 200,
    reg_write_u32 = 201,
    scan_find_u32 = 300,
    scan_replace_u32 = 301,
    scan_reset_u32= 302,
};

class dm_cmd
{
    public:    
        dm_cmd_type type;
        bool attached;
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
        std::atomic<bool> sem;
};

class dm_cmd_exit_cmd_loop : dm_cmd
{
    public:
        dm_cmd_exit_cmd_loop();
        ~dm_cmd_exit_cmd_loop();
};

class dm_cmd_proc_show : dm_cmd
{
    public:
        dm_cmd_proc_show();
        ~dm_cmd_proc_show();
};

class dm_cmd_proc_run : dm_cmd
{
    public:
        dm_cmd_proc_run(char const* const path);
        ~dm_cmd_proc_run();

        char* path;
};

class dm_cmd_proc_attach : dm_cmd
{
    public:
        dm_cmd_proc_attach(DWORD pid);
        ~dm_cmd_proc_attach();

        DWORD pid;
};

class dm_cmd_proc_start : dm_cmd
{
    public:
        dm_cmd_proc_start();
        ~dm_cmd_proc_start();
};

class dm_cmd_proc_stop : dm_cmd
{
    public:
        dm_cmd_proc_stop();
        ~dm_cmd_proc_stop();
};

class dm_cmd_reg_read_u32 : dm_cmd
{
    public:
        dm_cmd_reg_read_u32(UINT64 addr);
        ~dm_cmd_reg_read_u32();

        UINT64 addr;
};

class dm_cmd_reg_write_u32 : dm_cmd
{
    public:
        dm_cmd_reg_write_u32(UINT32 val, UINT64 addr);
        ~dm_cmd_reg_write_u32();

        UINT32 val;
        UINT64 addr;
};

class dm_cmd_scan_find_u32 : dm_cmd
{
    public:
        dm_cmd_scan_find_u32(UINT32 val);
        ~dm_cmd_scan_find_u32();

        UINT32 val;
};

class dm_cmd_scan_replace_u32 : dm_cmd
{
    public:
        dm_cmd_scan_replace_u32(UINT32 val);
        ~dm_cmd_scan_replace_u32();

        UINT32 val;
};

class dm_cmd_scan_reset_u32 : dm_cmd
{
    public:
        dm_cmd_scan_reset_u32();
        ~dm_cmd_scan_reset_u32();
};



