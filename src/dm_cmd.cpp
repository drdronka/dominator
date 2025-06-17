#include <stdio.h>
#include <windows.h>

#include "dm_cmd.h"
#include "dm_log.h"
#include "dm_core.h"

#define CMD_SIZE 256

dm_cmd::dm_cmd(dm_log* log_i, dm_core* core_i)
{
    log = log_i;
    core = core_i;
}

dm_cmd::dm_cmd(dm_log* log_i, dm_core* core_i, const char* path)
{
    log = log_i;
    core = core_i;
    core->start_process(path);
}

dm_cmd::~dm_cmd()
{
    delete core;
}

void dm_cmd::console()
{
    UINT8 loop_exit = 0;

    log->info("dominator cmd line");

    while(!loop_exit)
    {
        char cmd[CMD_SIZE];
        char buff[CMD_SIZE];

        fgets(cmd, CMD_SIZE, stdin);
        cmd[strlen(cmd)-1]=0;

        if(!strncmp(cmd, "run", strlen("run")))
        {
            if(get_arg(cmd, 1, buff) == dm_cmd_err::ok)
            {
                core->start_process(buff);
            }
        }
        else if(!strncmp(cmd, "ll", strlen("ll")))
        {
            if(get_arg(cmd, 1, buff) == dm_cmd_err::ok)
            {
                log->set_level((dm_log_level)atoi(buff));
            }
        }
        else if(!strncmp(cmd, "lf", strlen("lf")))
        {
            if(get_arg(cmd, 1, buff) == dm_cmd_err::ok)
            {
                log->set_format((dm_log_format)atoi(buff));
            }
        }   
        else if(!strncmp(cmd, "help", strlen("help")))
        {
            if(log->get_level() < dm_log_level::info)
            {
                log->set_level(dm_log_level::info);
                log->info("log level set [%d]", dm_log_level::info);
            }
            
            log->info("commands:");
            log->info("run <path>  - start process");
            log->info("ll <level>  - set log level (0 none, 1 error, 2 info, 3 debug)");
            log->info("lf <format> - set log format (0 clean, 1 with prefix)");
            log->info("help        - this info");
            log->info("exit        - stop/detach and exit");
        }
        else if(!strncmp(cmd, "exit", strlen("exit")))
        {
            loop_exit = 1;
        }
        else 
        {
            log->error("unknown command: [%s]", cmd);
        }
    }
}

dm_cmd_err dm_cmd::get_arg(char* cmd, UINT32 arg_n, char* arg)
{
    if(arg_n == 0)
    {
        UINT32 n = 0;
        
        while(cmd[n]  != ' ' && cmd[n++] != 0);

        strncpy(arg, cmd, n-1);        
        arg[n-1] = 0;

        log->debug("arg extracted [%s]", arg);

        return dm_cmd_err::ok;
    }
    else
    {
        UINT32 n = 0;
        
        while(cmd[++n] != 0) // strip white spaces
        {
            if(cmd[n] == ' ')
            {
                while(cmd[++n] == ' ');
                break;
            }
        }

        if(cmd[n] != 0)
        {
            return get_arg(cmd + n, arg_n - 1, arg);
        }
        else
        {
            log->error("too few arguments");
            return dm_cmd_err::get_arg;
        }
    }
}