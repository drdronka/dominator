#include <stdio.h>
#include <windows.h>

#include "dm_console.h"
#include "dm_log.h"
#include "dm_core.h"
#include "dm_cmd.h"

#define CMD_SIZE 256

dm_console::dm_console(dm_log* log, dm_core* core)
{
    this->log = log;
    this->core = core;
}

dm_console::~dm_console()
{
}

void dm_console::run()
{
    log->info("dominator cmd line");

    core->start_cmd_loop();
    core->stop_cmd_loop();
    core->start_cmd_loop();

    bool loop_exit = false;
    while(!loop_exit)
    {
        char input[CMD_SIZE];
        char cmd[CMD_SIZE];
        char arg[CMD_SIZE];

        fgets(input, CMD_SIZE, stdin);

        if(!get_arg(input, 0, cmd))
        {
            log->error("failed to extract cmd form input");
            continue;
        }

        if(is_arg(cmd, "run"))
        {
            if(get_arg(input, 1, arg))
            {
                core->add_cmd((dm_cmd*)new dm_cmd_start_process(arg));
            }
        }
        else if(is_arg(input, "ll"))
        {
            if(get_arg(input, 1, arg))
            {
                log->set_level((dm_log_level)strtol(arg, NULL, 0));
            }
        }
        else if(is_arg(input, "lf"))
        {
            if(get_arg(input, 1, arg))
            {
                log->set_format((dm_log_format)strtol(arg, NULL, 0));
            }
        }   
        else if(is_arg(input, "help"))
        {
            if(log->get_level() < dm_log_level::info)
            {
                log->set_level(dm_log_level::info);
            }
            
            log->info("commands:");
            log->info("run <path>  - start process");
            log->info("ll <level>  - set log level (0 none, 1 error, 2 info, 3 debug)");
            log->info("lf <format> - set log format (0 clean, 1 with prefix)");
            log->info("help        - this help page");
            log->info("exit        - stop/detach and exit");
        }
        else if(is_arg(cmd, "exit"))
        {
            core->stop_cmd_loop();
            loop_exit = true;
        }
        else 
        {
            log->error("unknown command: [%s]", cmd);
        }
    }
}

bool dm_console::get_arg(char const* const cmd, UINT32 arg_n, char* arg)
{
    if(arg_n == 0)
    {
        UINT32 n = 0;
        
        while(cmd[n] != ' ' && cmd[n] != 0 && cmd[n] != '\n')
        {
            n++;
        }

        strncpy(arg, cmd, n);        
        arg[n] = 0;

        log->debug("arg extracted [%s]", arg);

        return true;
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
            return false;
        }
    }
}

bool dm_console::is_arg(char const* const cmd, char const* const arg)
{
    if(!strncmp(cmd, arg, strlen(arg)))
    {
        return true;
    }

    return false;
}