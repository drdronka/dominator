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

    bool loop_exit = false;
    while(!loop_exit)
    {
        char input[CMD_SIZE];
        char cmd[CMD_SIZE];
        char arg1[CMD_SIZE];
        char arg2[CMD_SIZE];

        fgets(input, CMD_SIZE, stdin);

        if(!get_arg(input, 0, cmd))
        {
            log->error("failed to extract cmd form input");
            continue;
        }

        if(is_arg(cmd, "run"))
        {
            strip(input + strlen("run "), arg1);
            core->add_cmd((dm_cmd*)new dm_cmd_start_process(arg1));
        }
        else if(is_arg(cmd, "ru32"))
        {
            if(get_arg(input, 1, arg1))
            {
                core->add_cmd((dm_cmd*)new dm_cmd_ru32(strtoull(arg1, NULL, 0)));
            }
        }
        else if(is_arg(cmd, "wu32"))
        {
            if(get_arg(input, 1, arg1) && get_arg(input, 2, arg2))
            {
                core->add_cmd((dm_cmd*)new dm_cmd_wu32(strtoul(arg2, NULL, 0), strtoull(arg1, NULL, 0)));
            }
        }
        else if(is_arg(cmd, "fu32"))
        {
            if(get_arg(input, 1, arg1))
            {
                core->add_cmd((dm_cmd*)new dm_cmd_fu32(strtoul(arg1, NULL, 0)));
            }
        }
        else if(is_arg(cmd, "fu32w"))
        {
            if(get_arg(input, 1, arg1))
            {
                core->add_cmd((dm_cmd*)new dm_cmd_fu32_replace(strtoul(arg1, NULL, 0)));
            }
        }
        else if(is_arg(cmd, "fu32r"))
        {
            core->add_cmd((dm_cmd*)new dm_cmd_fu32_reset());
        }
        else if(is_arg(cmd, "ll"))
        {
            if(get_arg(input, 1, arg1))
            {
                log->set_level((dm_log_level)strtoul(arg1, NULL, 0));
            }
        }
        else if(is_arg(cmd, "lf"))
        {
            if(get_arg(input, 1, arg1))
            {
                log->set_format((dm_log_format)strtoul(arg1, NULL, 0));
            }
        }   
        else if(is_arg(cmd, "help"))
        {
            if(log->get_level() < dm_log_level::info)
            {
                log->set_level(dm_log_level::info);
            }
            
            log->info("--[ CORE ]--------------------------------------------------");
            log->info("run <path>          - start process");
            log->info("--[ REGS ]--------------------------------------------------");
            log->info("ru32 <addr64>       - read u32 value from target memory");
            log->info("wu32 <addr64> <val> - write u32 value to target memory");
            log->info("--[ MEM SCAN ]-----------------------------------------------");
            log->info("fu32 <val>          - find u32 value in target memory");
            log->info("                      subsequent calls only check previusly found addresses");
            log->info("fu32w <val>         - write to all addresses found by fu32");
            log->info("fu32r               - reset fu32 address list");
            log->info("--[ LOGS ]---------------------------------------------------");
            log->info("ll <level>          - set log level (0 none, 1 error, 2 info, 3 debug)");
            log->info("lf <format>         - set log format (0 clean, 1 with prefix)");
            log->info("--[ CMDLINE ]-------------------------------------------------");
            log->info("help                - this help page");
            log->info("exit                - kill/detach and exit");
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

        log->debug("console: arg extracted [%s]", arg);

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
    if(strlen(cmd) == strlen(arg) && !strncmp(cmd, arg, strlen(arg)))
    {
        return true;
    }

    return false;
}

void dm_console::strip(char const* const in, char* out)
{
    UINT32 i = 0;
    for(UINT32 n = 0; n < strlen(in); n++)
    {
        if(in[n] != '\"' && in[n] != '\n')
        {
            out[i] = in[n];
            i++;
        }
    }
    out[i] = 0;
}